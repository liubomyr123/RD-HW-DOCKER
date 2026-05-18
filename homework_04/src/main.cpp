#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>

#define TICKS_PER_REVOLUTION 1024
#define WHEEL_RADIUS_M 0.3f
#define WHEELBASE_M 1.0f

#define ENABLE_LOG	1
#define ENABLE_LOG_DEBUG  0

class Logger {
private:
    std::ofstream file;

    Logger() {
        file.open("app.log", std::ios::out | std::ios::trunc);
    }

public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void log(const std::string& type, const std::string& msg) {
        std::string line = type + " " + msg;

        std::cout << line << "\n";
        file << line << "\n";
    }
};

#if ENABLE_LOG
    #define LOG_INFO(x) \
        do { \
            std::ostringstream oss; \
            oss << x; \
            Logger::instance().log("", oss.str()); \
        } while(0)

    #define LOG_PROCESS(x) \
        do { \
            std::ostringstream oss; \
            oss << x; \
            Logger::instance().log("🔄", oss.str()); \
        } while(0)

    #define LOG_ERROR(x) \
        do { \
            std::ostringstream oss; \
            oss << x; \
            Logger::instance().log("❌", oss.str()); \
        } while(0)

    #define LOG_WARN(x) \
        do { \
            std::ostringstream oss; \
            oss << x; \
            Logger::instance().log("⚠️ ", oss.str()); \
        } while(0)

    #define LOG_SUCCESS(x) \
        do { \
            std::ostringstream oss; \
            oss << x; \
            Logger::instance().log("✅", oss.str()); \
        } while(0)
#else
    #define LOG_INFO(x)
    #define LOG_PROCESS(x)
    #define LOG_ERROR(x)
    #define LOG_WARN(x)
    #define LOG_SUCCESS(x)
#endif

#if ENABLE_LOG_DEBUG
    #define LOG_DEBUG(x) \
        do { \
            std::ostringstream oss; \
            oss << x; \
            Logger::instance().log("🐞", oss.str()); \
        } while(0)
#else
    #define LOG_DEBUG(x)
#endif

struct InputDataLine
{
    long timestamp_ms;
    long fl_ticks;
    long fr_ticks; 
    long bl_ticks; 
    long br_ticks;
};

struct InputData 
{
    std::vector<InputDataLine> lines;
};

struct OutputDataLine
{
    double timestamp_ms;
    double x;
    double y; 
    double theta; 
};

struct OutputData 
{
    std::vector<OutputDataLine> lines;
};

bool getInputData(const std::string& file_name, InputData& inputData)
{
    LOG_PROCESS("Reading " + file_name + "...");

    std::ifstream file(file_name);
    if (!file) {
        LOG_ERROR("Error opening file");
        return false;
    }

    long timestamp_ms;
    long fl_ticks, fr_ticks, bl_ticks, br_ticks;

    while (file 
            >> timestamp_ms 
            >> fl_ticks 
            >> fr_ticks 
            >> bl_ticks 
            >> br_ticks) {
        InputDataLine line{};
        line.timestamp_ms = timestamp_ms;
        line.fl_ticks = fl_ticks;
        line.fr_ticks = fr_ticks;
        line.bl_ticks = bl_ticks;
        line.br_ticks = br_ticks;
        inputData.lines.push_back(line);
    }

    if (file.bad()) {
        LOG_ERROR("IO error");
        return false;
    }

    file.close();

    LOG_SUCCESS("Successfully read all data");

    return true;
}

bool getOutputData(OutputData& outputData, const InputData& inputData)
{
    double x = 0, y = 0, theta = 0;
    for (size_t i = 0; i < inputData.lines.size(); i++) 
    {
        if (i == 0) // Починаємо розрахунки з 2-го елемента. Перший має усі нулі, пропускаємо
        {
            continue;
        }

        /**
         *  Крок 1. Рiзниця iмпульсiв по кожному колесу:
         *  d_fl = fl_ticks[i] - fl_ticks[i-1]
         *  d_fr = fr_ticks[i] - fr_ticks[i-1]
         *  d_bl = bl_ticks[i] - bl_ticks[i-1]
         *  d_br = br_ticks[i] - br_ticks[i-1]
         */
        double d_fl = inputData.lines[i].fl_ticks - inputData.lines[i-1].fl_ticks;
        double d_fr = inputData.lines[i].fr_ticks - inputData.lines[i-1].fr_ticks;
        double d_bl = inputData.lines[i].bl_ticks - inputData.lines[i-1].bl_ticks;
        double d_br = inputData.lines[i].br_ticks - inputData.lines[i-1].br_ticks;

        /**
         *  Крок 2. Усереднити борти (передне i заднє колесо одного боку обертаються синхронно):
         * d_left  = (d_fl + d_bl) / 2
         * d_right = (d_fr + d_br) / 2
         */
        double d_left = (d_fl + d_bl) / 2;
        double d_right = (d_fr + d_br) / 2;

        /**
         *  Крок 3. Перевести iмпульси у метри:
         * distance_per_tick = 2 * pi * wheel_radius_m / ticks_per_revolution
         * dL = d_left  * distance_per_tick
         * dR = d_right * distance_per_tick
         */
        double distance_per_tick = 2 * M_PI * WHEEL_RADIUS_M / TICKS_PER_REVOLUTION;
        double dL = d_left * distance_per_tick;
        double dR = d_right * distance_per_tick;

        /**
         *  Крок 4. Скiльки пройшов центр робота i на скiльки повернувся:
         * d      = (dL + dR) / 2              // пройдена вiдстань центру
         * dtheta = (dR - dL) / wheelbase_m    // змiна орiєнтацiї
         */
        double d = (dL + dR) / 2;
        double dtheta = (dR - dL) / WHEELBASE_M;

        /**
         *  Крок 5. Оновити позицiю через усереднений напрямок на кроцi:
         * x     += d * cos(theta + dtheta / 2)
         * y     += d * sin(theta + dtheta / 2)
         * theta += dtheta
         */
        x += d * cos(theta + dtheta / 2);
        y += d * sin(theta + dtheta / 2);
        theta += dtheta;

        OutputDataLine line{};
        line.timestamp_ms = inputData.lines[i].timestamp_ms;
        line.x = x;
        line.y = y;
        line.theta = theta;
        outputData.lines.push_back(line);
    }
    return true;
}

bool writeOutputToFile(const std::string& file_name, const OutputData& outputData)
{
    LOG_PROCESS("Writing result to " << file_name << "...");

    std::ofstream file(file_name);
    if (!file)
    {
        LOG_ERROR("Error opening file");
        return false;
    }

    for (auto line: outputData.lines) 
    {
        file << line.timestamp_ms << " " << line.x << " " << line.y << " " << line.theta << "\n";
        LOG_INFO(line.timestamp_ms << " " << line.x << " " << line.y << " " << line.theta);
    }

    if (file.bad()) {
        LOG_ERROR("IO error");
        return false;
    }

    file.close();

    LOG_SUCCESS("Successfully wrote result to file");

    return true;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: ugv_odometry <input_path>\n";
        return 1;
    }

    std::string output_file_path = "output.txt";
    std::string input_file_path{};
    for (int i = 0; i < argc; ++i) {
        if (i == 1)
        {
            input_file_path = argv[i];
        }
    }

    InputData inputData{};
    if (!getInputData(input_file_path, inputData))
    {
        return 1;
    }

    OutputData outputData{};
    if (!getOutputData(outputData, inputData)) 
    {
        return 1;
    }

    if (!writeOutputToFile(output_file_path, outputData)) 
    {
        return 1;
    }

    return 0;
}
