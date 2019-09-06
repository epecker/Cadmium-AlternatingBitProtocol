/**
* Cristina Ruiz Martin
* ARSLab - Carleton University
*
* Subnet:
* Cadmium implementation of CD++ Subnet atomic model
*/

#ifndef _SUBNET_HPP__
#define _SUBNET_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>
#include <random>

#include "../data_structures/message.hpp"

using namespace cadmium;
using namespace std;

//Port definition
struct Subnet_defs{
    struct out : public out_port<Message_t> {};
    struct in : public in_port<Message_t> {};
};

template<typename TIME> class Subnet{
    using defs=Subnet_defs; // putting definitions in context
    public:
    // ports definition
    using input_ports=tuple<typename defs::in>;
    using output_ports=tuple<typename defs::out>;
    // state definition
    struct state_type{
        bool transmitting;
        Message_t packet;
        int index;
    }; 
    state_type state;    
    // default constructor
    Subnet() noexcept{
        state.transmitting = false;
        state.index        = 0;
    }     
    // internal transition
    void internal_transition() {
        state.transmitting = false;  
    }
    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) { 
        state.index ++;
        if(get_messages<typename defs::in>(mbs).size()>1) assert(false && "One message at a time");                
        for (const auto &x : get_messages<typename defs::in>(mbs)) {
            state.packet = x;
            state.transmitting = true; 
        }               
    }
    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), move(mbs));
    }
    // output function
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        if ((double)rand() / (double) RAND_MAX  < 0.95){                
            get_messages<typename defs::out>(bags).push_back(state.packet);
        }
        return bags;
    }
    // time_advance function
    TIME time_advance() const {
        TIME next_internal;
        default_random_engine generator;
        normal_distribution<double> distribution(3.0, 1.0); 
        if (state.transmitting) {
            initializer_list<int> time = {0, 0, static_cast < int > (round(distribution(generator)))};
            // time is hour min and second
            next_internal = TIME(time);
        }else {
            next_internal = numeric_limits<TIME>::infinity();
        }    
        return next_internal;
    }

    friend ostringstream& operator<<(ostringstream& os, const typename Subnet<TIME>::state_type& i) {
        os << "index: " << i.index << " & transmitting: " << i.transmitting; 
        return os;
    }
};    
#endif // _SUBNET_HPP__