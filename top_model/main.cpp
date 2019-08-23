//C++ headers
#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>

//Cadmium Simulator headers
#include <cadmium/modeling/coupling.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/tuple_to_ostream.hpp>
#include <cadmium/logger/common_loggers.hpp>


#include "../vendor/NDTime.hpp"
#include "../vendor/iestream.hpp"

#include "../data_structures/message.hpp"

#include "atomic_headers.hpp"


using namespace std;
using namespace cadmium;

using hclock=chrono::high_resolution_clock;
using TIME = NDTime;



/***** SETING INPUT PORTS FOR COUPLEDs *****/
struct inp_control : public in_port<Message_t>{};
struct inp_1 : public in_port<Message_t>{};
struct inp_2 : public in_port<Message_t>{};
/***** SETING OUTPUT PORTS FOR COUPLEDs *****/
struct outp_ack : public out_port<Message_t>{};
struct outp_1 : public out_port<Message_t>{};
struct outp_2 : public out_port<Message_t>{};
struct outp_pack : public out_port<Message_t>{};


/********************************************/
/****** APPLICATION GENERATOR *******************/
/********************************************/
template<typename T>
class ApplicationGen : public iestream_input<Message_t,T> {
public:
  ApplicationGen() = default;
  ApplicationGen(const char* file_path) : iestream_input<Message_t,T>(file_path) {}
};


int main(int argc, char ** argv) {

  if (argc < 2) {
   cout << "you are using this program with wrong parameters. Te program should be invoked as follow:";
   cout << argv[0] << " path to the input file " << endl;
   return 1; 
  }

  auto start = hclock::now(); //to measure simulation execution time

/*************** Loggers *******************/
  static ofstream out_data("../simulation_results/abp_output.txt");
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



/********************************************/
/****** APPLICATION GENERATOR *******************/
/********************************************/
string input_data_control = argv[1];
const char * i_input_data_control = input_data_control.c_str();

shared_ptr<dynamic::modeling::model> generator_con = dynamic::translate::make_dynamic_atomic_model<ApplicationGen, TIME, const char* >("generator_con" , move(i_input_data_control));


/********************************************/
/****** SENDER *******************/
/********************************************/

shared_ptr<dynamic::modeling::model> sender1 = dynamic::translate::make_dynamic_atomic_model<Sender, TIME>("sender1");

/********************************************/
/****** RECIEVER *******************/
/********************************************/

shared_ptr<dynamic::modeling::model> receiver1 = dynamic::translate::make_dynamic_atomic_model<Receiver, TIME>("receiver1");



/********************************************/
/****** SUBNET *******************/
/********************************************/

shared_ptr<dynamic::modeling::model> subnet1 = dynamic::translate::make_dynamic_atomic_model<Subnet, TIME>("subnet1");
shared_ptr<dynamic::modeling::model> subnet2 = dynamic::translate::make_dynamic_atomic_model<Subnet, TIME>("subnet2");

/************************/
/*******NETWORK********/
/************************/
dynamic::modeling::Ports iports_Network = {typeid(inp_1),typeid(inp_2)};
dynamic::modeling::Ports oports_Network = {typeid(outp_1),typeid(outp_2)};
dynamic::modeling::Models submodels_Network = {subnet1, subnet2};
dynamic::modeling::EICs eics_Network = {
  dynamic::translate::make_EIC<inp_1, Subnet_defs::in>("subnet1"),
  dynamic::translate::make_EIC<inp_2, Subnet_defs::in>("subnet2")
};
dynamic::modeling::EOCs eocs_Network = {
  dynamic::translate::make_EOC<Subnet_defs::out,outp_1>("subnet1"),
  dynamic::translate::make_EOC<Subnet_defs::out,outp_2>("subnet2")
};
dynamic::modeling::ICs ics_Network = {};
shared_ptr<dynamic::modeling::coupled<TIME>> NETWORK = make_shared<dynamic::modeling::coupled<TIME>>(
 "Network", 
 submodels_Network, 
 iports_Network, 
 oports_Network, 
 eics_Network, 
 eocs_Network, 
 ics_Network 
  );

/************************/
/*******ABPSimulator********/
/************************/
dynamic::modeling::Ports iports_ABPSimulator = {typeid(inp_control)};
dynamic::modeling::Ports oports_ABPSimulator = {typeid(outp_ack),typeid(outp_pack)};
dynamic::modeling::Models submodels_ABPSimulator = {sender1, receiver1,NETWORK};
dynamic::modeling::EICs eics_ABPSimulator = {
  cadmium::dynamic::translate::make_EIC<inp_control, Sender_defs::controlIn>("sender1")
};
dynamic::modeling::EOCs eocs_ABPSimulator = {
  dynamic::translate::make_EOC<Sender_defs::packetSentOut,outp_pack>("sender1"),
dynamic::translate::make_EOC<Sender_defs::ackReceivedOut,outp_ack>("sender1")
};
dynamic::modeling::ICs ics_ABPSimulator = {
  dynamic::translate::make_IC<Sender_defs::dataOut, inp_1>("sender1","Network"),
  dynamic::translate::make_IC<outp_2, Sender_defs::ackIn>("Network","sender1"),
  dynamic::translate::make_IC<Receiver_defs::out, inp_2>("receiver1","Network"),
  dynamic::translate::make_IC<outp_1, Receiver_defs::in>("Network","receiver1")
};
shared_ptr<dynamic::modeling::coupled<TIME>> ABPSimulator = make_shared<dynamic::modeling::coupled<TIME>>(
 "ABPSimulator", 
 submodels_ABPSimulator, 
 iports_ABPSimulator, 
 oports_ABPSimulator, 
 eics_ABPSimulator, 
 eocs_ABPSimulator, 
 ics_ABPSimulator 
  );


/************************/
/*******TOP MODEL********/
/************************/
dynamic::modeling::Ports iports_TOP = {};
dynamic::modeling::Ports oports_TOP = {typeid(outp_pack),typeid(outp_ack)};
dynamic::modeling::Models submodels_TOP = {generator_con, ABPSimulator};
dynamic::modeling::EICs eics_TOP = {};
dynamic::modeling::EOCs eocs_TOP = {
  dynamic::translate::make_EOC<outp_pack,outp_pack>("ABPSimulator"),
dynamic::translate::make_EOC<outp_ack,outp_ack>("ABPSimulator")
};
dynamic::modeling::ICs ics_TOP = {
  dynamic::translate::make_IC<iestream_input_defs<Message_t>::out,inp_control>("generator_con","ABPSimulator")
};
shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> TOP = make_shared<dynamic::modeling::coupled<TIME>>(
 "TOP", 
 submodels_TOP, 
 iports_TOP, 
 oports_TOP, 
 eics_TOP, 
 eocs_TOP, 
 ics_TOP 
  );

///****************////

    auto elapsed1 = chrono::duration_cast<chrono::duration<double, ratio<1>>>(hclock::now() - start).count();
    cout << "Model Created. Elapsed time: " << elapsed1 << "sec" << endl;
    
    dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
    elapsed1 = chrono::duration_cast<chrono::duration<double, ratio<1>>>(hclock::now() - start).count();
    cout << "Runner Created. Elapsed time: " << elapsed1 << "sec" << endl;

    cout << "Simulation starts" << endl;

    r.run_until(NDTime("04:00:00:000"));
    auto elapsed = chrono::duration_cast<chrono::duration<double, std::ratio<1>>>(hclock::now() - start).count();
    cout << "Simulation took:" << elapsed << "sec" << endl;
    return 0;
}
