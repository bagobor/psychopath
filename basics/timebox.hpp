#ifndef TIMEBOX_HPP
#define TIMEBOX_HPP

#include "numtype.h"

#include <iostream>
#include <stdlib.h>

template <class T>
class TimeBox
{
public:
	T *states;
	uint8_t state_count;


	TimeBox();
	~TimeBox();

	// Initializes the timebox with the given number of states
	bool init(const uint8_t &state_count_);

	// Given a time in range [0.0, 1.0], fills in the state indices on
	// either side along with an alpha to blend between them.
	// Returns true on success, false on failure.  Failure typically
	// means that there is only one state in the TimeBox.
	bool query_time(const float &time, int32_t *ia, int32_t *ib, float *alpha) const {
		if (state_count < 2)
			return false;

		if (time < 1.0) {
			const float temp = time * (state_count - 1);
			const int32_t index = temp;
			*ia = index;
			*ib = index + 1;
			*alpha = temp - (float)(index);
		} else {
			*ia = state_count - 2;
			*ib = state_count - 1;
			*alpha = 1.0;
		}

		return true;
	}

	// Allows transparent access to the underlying state data
	T &operator[](const int32_t &i) {
		return states[i];
	}

	const T &operator[](const int32_t &i) const {
		return states[i];
	}

	size_t size() const {
		return state_count;
	}
};


template <class T>
TimeBox<T>::TimeBox()
{
	states = nullptr;
	state_count = 0;
}

template <class T>
TimeBox<T>::~TimeBox()
{
	if (states)
		delete [] states;
}


template <class T>
bool TimeBox<T>::init(const uint8_t &state_count_)
{
	if (state_count == state_count_) {
		return true;
	} else if (states) {
		delete [] states;
	}

	states = new T[state_count_];
	state_count = state_count_;
	return true;
}







#endif
