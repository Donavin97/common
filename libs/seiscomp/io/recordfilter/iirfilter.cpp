/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT RecordFilter_IIR

#include <seiscomp/logging/log.h>
#include <seiscomp/io/recordfilter/iirfilter.h>


namespace Seiscomp {
namespace IO {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

template <typename T>
Array::DataType dataType() {
	throw Core::TypeConversionException("RecordFilter::IIR: wrong data type");
}


template <>
Array::DataType dataType<float>() {
	return Array::FLOAT;
}


template <>
Array::DataType dataType<double>() {
	return Array::DOUBLE;
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordIIRFilter<T>::RecordIIRFilter(InplaceFilterType *filter)
: _filter(filter) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordIIRFilter<T>::~RecordIIRFilter() {
	if ( _filter != NULL )
		delete _filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordIIRFilter<T> &
RecordIIRFilter<T>::operator=(RecordIIRFilter<T>::InplaceFilterType *f) {
	setIIR(f);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void RecordIIRFilter<T>::setIIR(Math::Filtering::InPlaceFilter<T> *f) {
	if ( _filter != NULL )
		delete _filter;

	_lastEndTime = Core::Time();
	_filter = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
Record *RecordIIRFilter<T>::feed(const Record *rec) {
	const Array *data = rec->data();
	if ( data == NULL ) return NULL;

	TypedArray<T> *tdata = (TypedArray<T>*)data->copy(dataType<T>());
	if ( tdata == NULL ) return NULL;

	// Copy the record and assign the data
	GenericRecord *out = new GenericRecord(*rec);
	out->setData(tdata);

	apply(out);

	return out;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool RecordIIRFilter<T>::apply(GenericRecord *rec) {
	// No filter is no error
	if ( _filter == NULL ) return true;

	if ( rec->dataType() != dataType<T>() ) {
		throw Core::TypeConversionException("RecordFilter::IIR: wrong data type");
	}

	if ( _lastEndTime.valid() ) {
		// If the sampling frequency changed, reset the filter
		if ( _samplingFrequency != rec->samplingFrequency() ) {
			SEISCOMP_WARNING("[%s] sps change (%f != %f): reset filter",
			                 rec->streamID().c_str(), _samplingFrequency,
			                 rec->samplingFrequency());
			reset();
		}
		else {
			Core::TimeSpan diff = rec->startTime() - _lastEndTime;
			// Overlap or gap does not matter, we need to reset the filter
			// for non-continuous records
			if ( fabs(diff) > (0.5/_samplingFrequency) ) {
//				SEISCOMP_DEBUG("[%s] discontinuity of %fs: reset filter",
//				               rec->streamID().c_str(), (double)diff);
				reset();
			}
		}
	}

	if ( !_lastEndTime.valid() ) {
		// First call after construction or reset: initialize
		_lastEndTime = rec->endTime();
		_samplingFrequency = rec->samplingFrequency();
		_filter->setSamplingFrequency(_samplingFrequency);
		_filter->setStreamID(rec->networkCode(), rec->stationCode(), rec->locationCode(), rec->channelCode());
	}

	TypedArray<T> *data = (TypedArray<T>*)rec->data();
	_filter->apply(*data);
	_lastEndTime = rec->endTime();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
Record *RecordIIRFilter<T>::flush() {
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void RecordIIRFilter<T>::reset() {
	// Clone the filter
	if ( _filter ) {
		InplaceFilterType *tmp = _filter;
		_filter = _filter->clone();
		delete tmp;
	}

	// Reset last end time
	_lastEndTime = Core::Time();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordFilterInterface* RecordIIRFilter<T>::clone() const {
	return new RecordIIRFilter<T>(_filter != NULL?_filter->clone():NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template class SC_SYSTEM_CORE_API RecordIIRFilter<float>; \
template class SC_SYSTEM_CORE_API RecordIIRFilter<double>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}