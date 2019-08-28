/**
* Cristina Ruiz Martin
* ARSLab - Carleton University
*
* This file contains all the loger code and the call to the simulator
*
*/

#ifndef __LOGGER_SIMULATOR_INCLUDE_HPP__
#define __LOGGER_SIMULATOR_INCLUDE_HPP__

/*************** Loggers *******************/
  static ofstream out_data(SIMULATION_OUTPUT_PATH);
    struct oss_sink_provider{
        static ostream& sink(){          
            return out_data;
        }
    };
using info=logger::logger<cadmium::logger::logger_info, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
using debug=logger::logger<cadmium::logger::logger_debug, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
using state=logger::logger<cadmium::logger::logger_state, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
using log_messages=logger::logger<cadmium::logger::logger_messages, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
using routing=logger::logger<cadmium::logger::logger_message_routing, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
using global_time=logger::logger<cadmium::logger::logger_global_time, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
using local_time=logger::logger<cadmium::logger::logger_local_time, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
using log_all=logger::multilogger<info, debug, state, log_messages, routing, global_time, local_time>;
using logger_top=logger::multilogger<log_messages, global_time>;
/*******************************************/
/************ Simulator ********************/
auto elapsed1 = chrono::duration_cast<chrono::duration<double, ratio<1>>>(hclock::now() - start).count();
cout << "Model Created. Elapsed time: " << elapsed1 << "sec" << endl;
    
dynamic::engine::runner<TIME, LOGGER> r(TOP_MODEL, INTIAL_TIME);
elapsed1 = chrono::duration_cast<chrono::duration<double, ratio<1>>>(hclock::now() - start).count();
cout << "Runner Created. Elapsed time: " << elapsed1 << "sec" << endl;

cout << "Simulation starts" << endl;

r.run_until(END_TIME);
auto elapsed = chrono::duration_cast<chrono::duration<double, std::ratio<1>>>(hclock::now() - start).count();
cout << "Simulation took:" << elapsed << "sec" << endl;

/*****************************************/
#endif //__LOGGER_SIMULATOR_INCLUDE_HPP__
