#include "util/sdk.h"
#include <boost/asio/signal_set.hpp>
#include <thread>
#include <algorithm>

#include "json/json_loader.h"
#include "http/request_handler.h"
#include "logger/logging_request_handler.h"
#include "app/tick.h"
#include "util/comand_line.h"  
#include "logger/logger.h"   
#include "json/extra_data.h"   
#include "domain/loot_generator.h"   
#include "domain/model_serialization.h"
#include "db/db.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;

namespace {

constexpr const char DB_URL_ENV_NAME[]{"GAME_DB_URL"};
 
struct AppParam{
    std::string config_file;
    std::string static_content_path;
    bool randomize_spawn_points;
    int save_state_period;
    std::string state_file;
    int tick_period;
    std::string db_url;
};

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

AppParam GetAppParam(const comand_line::Args& args){
    AppParam param;
    param.config_file = args.config_file;
    param.static_content_path = args.www_root;
    param.randomize_spawn_points = args.randomize_spawn_points;
    param.save_state_period = args.save_state_period;
    param.state_file = args.state_file;
    param.tick_period = args.tick_period;
    if (const auto* url = std::getenv(DB_URL_ENV_NAME)) {
        param.db_url = url;
    } else {
        throw std::runtime_error(DB_URL_ENV_NAME + " environment variable not found"s);
    }
    return param;
}

AppParam GetAppParam(){
    AppParam param;
    param.config_file = "../../data/config.json";
    param.static_content_path = "../../static";
    param.randomize_spawn_points = true;
    param.save_state_period = 10;
    // param.state_file = "";
    param.state_file = "/home/anton/sprint4/tema1/6_state_serialization_/build/state_file_01.txt";
    param.tick_period = 0;
    param.db_url = "postgres://postgres:123@localhost:5432/anton_tast";
    return param;
}

 
}  // namespace

    int main(int argc, const char* argv[]) {
    logger::InitLogFilter();
    try {
        auto args = comand_line::ParseCommandLine(argc, argv);
        if (!args) {
            return EXIT_SUCCESS;
        }
        auto app_param = GetAppParam(*args);
        // auto app_param = GetAppParam();

        // 1. Загружаем карты и настройки из файла и строим модель игры
        model::Game game(app_param.randomize_spawn_points);
        extra_data::ExtraData ext_data;
        json_loader::LoadGame(game, app_param.config_file, ext_data);

        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {ioc.stop();}
        });
        // Инициализируем базу данных 
        db::Database data_base{app_param.db_url, std::min(num_threads, unsigned(4))};

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры через объект с сценариями игры (application)
        // strand для выполнения запросов к API
        app::Application application(game, data_base.GetUnitOfWorkFactory());
        game.SetRetireListener(application.GetRecordsUseCase());
        serialization::SerializingListener serializer(std::chrono::milliseconds{app_param.save_state_period}
            , app_param.state_file, application);
        if (app_param.save_state_period > 0 && !app_param.state_file.empty()) {
            application.SetListener(&serializer);
        }
//--------------------------------------------------------------------------------
        // {
        //     std::string map = "map1";
        //     std::string player_name = "pl";
        //     std::string directs = "UDLR";
        //     std::vector<std::string> tokens;
        //     int player_count = 500;
        //     tokens.reserve(player_count);

        //     std::random_device random_device_;
        //     std::uniform_int_distribution<int> chr_idx(0, directs.size()-1);


        //     for (int i = 0; i < player_count; ++i) {
        //         auto pl = application.AddPlayer(player_name+"_"+std::to_string(i), map);
        //         tokens.push_back(*pl.token);
        //     }

        //     for (const auto& token : tokens) {
        //         char random_direct = directs[chr_idx(random_device_)];
        //         application.SetDogDirect(token, random_direct);
        //     }
        // }

//--------------------------------------------------------------------------------
        auto api_strand = net::make_strand(ioc);
        const bool tick_mode = app_param.tick_period > 0; 
        auto handler = std::make_shared<http_handler::RequestHandler>(application, app_param.static_content_path, api_strand, !tick_mode, ext_data);
        log_handler::LoggingRequestHandler logging_handler{*handler};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;

        http_server::ServeHttp(ioc, {address, port}
        , [&logging_handler](auto&& end_point, auto&& req, auto&& send) {
            logging_handler(std::forward<decltype(end_point)>(end_point)
            , std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });

        // 6. Настраиваем вызов метода Application::Tick каждые хх миллисекунд внутри strand
        if(tick_mode){
            auto ticker = std::make_shared<tick::Ticker>(api_strand, std::chrono::milliseconds(app_param.tick_period),
                [&application](std::chrono::milliseconds delta) { application.ChangeGameState(delta); }
            );
            ticker->Start();
        }

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        logger::LogServerStarted(port, address.to_string());
        // Загружаем состояние игры из файла если указан файл
        if (serializer.StateFileNotEmpty()) {
            serializer.RestoreGameState();
        }
        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
        // 7. Сохраняем состояние игры в файл если указан файл для сохранения
        if (serializer.StateFileNotEmpty()) {
            serializer.SerializeGameState();
            BOOST_LOG_TRIVIAL(info) << "Serialize Game State on exit"sv;
        }
    } catch (const std::exception& ex) {
        logger::LogExitFailure(ex); 
        return EXIT_FAILURE;
    }
    logger::LogServerExited();

}
