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


#include <seiscomp/math/geo.h>
#include <seiscomp/processing/magnitudes/MLv.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "mm";

}


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_MLv, MagnitudeProcessor, "MagnitudeProcessor_MLv");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_MLv, "MLv");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_MLv::MagnitudeProcessor_MLv()
 : MagnitudeProcessor("MLv") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_MLv::setup(const Settings &settings) {
	MagnitudeProcessor::setup(settings);
	std::string logA0;

	// This is the default
	logA0 = "0 -1.3;60 -2.8;100 -3.0;400 -4.5;1000 -5.85";
	maxDistanceKm = -1; // distance according to the logA0 range

	try { logA0 = settings.getString("magnitudes.MLv.logA0"); }
	catch ( ... ) {}
	try {
		logA0 = settings.getString("MLv.logA0");
		SEISCOMP_WARNING("MLv.logA0 has been depreciated");
		SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.MLv.logA0");
	}
	catch ( ... ) {}

	logA0_dist.clear(); logA0_val.clear();
	std::istringstream iss(logA0);
	std::string item;
	while ( getline(iss, item,';') ) {
		std::istringstream iss_item(item);
		double dist, val;
		iss_item >> dist >> val;
		logA0_dist.push_back(dist);
		logA0_val.push_back(val);
	}

	try { maxDistanceKm = settings.getDouble("magnitudes.MLv.maxDistanceKm"); }
	catch ( ... ) {}

	try {
		maxDistanceKm = settings.getDouble("MLv.maxDistanceKm");
		SEISCOMP_WARNING("MLv.maxDistanceKm has been depreciated");
		SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.MLv.maxDistanceKm");
	}
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MagnitudeProcessor_MLv::logA0(double dist_km) const {
	for ( size_t i=1; i<logA0_dist.size(); ++i ) {
		if ( logA0_dist[i-1] <= dist_km && dist_km <= logA0_dist[i] ) {
			double q = (dist_km-logA0_dist[i-1])/(logA0_dist[i]-logA0_dist[i-1]);
			return q*(logA0_val[i]-logA0_val[i-1])+logA0_val[i-1];
		}
	}

	throw Core::ValueException("distance out of range");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_MLv::computeMagnitude(
	double amplitude, const std::string &unit,
	double, double,
	double delta, double depth,
	const DataModel::Origin *,
	const DataModel::SensorLocation *,
	const DataModel::Amplitude *,
	double &value) {
	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	// Clip depth to 0
	if ( depth < 0 ) depth = 0;

	double distanceKm = Math::Geo::deg2km(delta);
	if ( maxDistanceKm > 0 && distanceKm > maxDistanceKm )
		return DistanceOutOfRange;

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) )
		return InvalidAmplitudeUnit;

	try {
		value = log10(amplitude) - logA0(distanceKm);
	}
	catch ( Core::ValueException & ) {
		return DistanceOutOfRange;
	}

	value = correctMagnitude(value);

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
