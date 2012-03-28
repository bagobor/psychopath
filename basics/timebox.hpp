#ifndef TIMEBOX_HPP
#define TIMEBOX_HPP

#include <iostream>
#include <stdlib.h>

template <class T>
class TimeBox {
    public:
        T *states;
        unsigned char state_count;
        
        
        TimeBox();
        ~TimeBox();
        
        // Initializes the timebox with the given number of states
        bool init(const unsigned char &state_count_);
        
        // Given a time in range [0.0, 1.0], fills in the state indices on
        // either side along with an alpha to blend between them.
        // Returns true on success, false on failure.  Failure typically
        // means that there is only one state in the TimeBox.
        bool query_time(const float &time, int *ia, int *ib, float *alpha) const
        {
            if(state_count < 2)
                return false;
            
            if(time < 1.0)
            {
                const float temp = time * (state_count - 1);
                const int index = temp;
                *ia = index;
                *ib = index + 1;
                *alpha = temp - (float)(index);
            }
            else
            {
                *ia = state_count - 2;
                *ib = state_count - 1;
                *alpha = 1.0;
            }
            
            return true;
        }
        
        // Allows transparent access to the underlying state data
        T &operator[](const int &i)
        {
                return states[i];
        }
        
        const T &operator[](const int &i) const
        {
                return states[i];
        }
};


template <class T>
TimeBox<T>::TimeBox()
{
    states = NULL;
    state_count = 0;
}

template <class T>
TimeBox<T>::~TimeBox()
{
    if(states)
        delete [] states;
}
        

template <class T>
bool TimeBox<T>::init(const unsigned char &state_count_)
{
    if(state_count == state_count_)
    {
        return true;
    }
    else if(states)
    {
        delete [] states;
    }
    
    states = new T[state_count_];
    state_count = state_count_;
    return true;
}







#endif