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


#ifndef SEISCOMP_DATAMODEL_DATABASEREADER_H__
#define SEISCOMP_DATAMODEL_DATABASEREADER_H__


#include <seiscomp/datamodel/object.h>
#include <seiscomp/datamodel/databasearchive.h>


namespace Seiscomp {
namespace DataModel {

class EventParameters;
class Pick;
class Comment;
class Amplitude;
class Reading;
class PickReference;
class AmplitudeReference;
class Origin;
class CompositeTime;
class Arrival;
class StationMagnitude;
class Magnitude;
class StationMagnitudeContribution;
class FocalMechanism;
class MomentTensor;
class DataUsed;
class MomentTensorPhaseSetting;
class MomentTensorStationContribution;
class MomentTensorComponentContribution;
class Catalog;
class Event;
class EventDescription;
class OriginReference;
class FocalMechanismReference;
class Config;
class ParameterSet;
class Parameter;
class ConfigModule;
class ConfigStation;
class Setup;
class QualityControl;
class QCLog;
class WaveformQuality;
class Outage;
class Inventory;
class StationGroup;
class StationReference;
class AuxDevice;
class AuxSource;
class Sensor;
class SensorCalibration;
class Datalogger;
class DataloggerCalibration;
class Decimation;
class ResponsePAZ;
class ResponseFIR;
class ResponseIIR;
class ResponsePolynomial;
class ResponseFAP;
class Network;
class Station;
class SensorLocation;
class AuxStream;
class Stream;
class Routing;
class Route;
class RouteArclink;
class RouteSeedlink;
class Access;
class Journaling;
class JournalEntry;
class ArclinkLog;
class ArclinkRequest;
class ArclinkStatusLine;
class ArclinkRequestLine;
class ArclinkUser;
class DataAvailability;
class DataExtent;
class DataSegment;
class DataAttributeExtent;

DEFINE_SMARTPOINTER(DatabaseReader);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief Database reader class for the scheme classes
 *  This class uses a database interface to read objects from a database.
 *  Different database backends can be implemented by creating a driver
 *  in seiscomp/services/database.
 */
class SC_SYSTEM_CORE_API DatabaseReader : public DatabaseArchive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		DatabaseReader(Seiscomp::IO::DatabaseInterface* dbDriver);

		//! Destructor
		~DatabaseReader();


	// ----------------------------------------------------------------------
	//  Read methods
	// ----------------------------------------------------------------------
	public:
		/**
		 * Loads the object with the given publicID from the
		 * database with all its children and subchildren.
		 * @param classType The type of the object
		 * @param publicID The publicID of the object
		 * @return An unmanaged pointer thats ownership goes
		 *         over to the caller. If no object has been found, NULL
		 *         is returned.
		 */
		PublicObject *loadObject(const Seiscomp::Core::RTTI& classType,
		                         const std::string& publicID);

		EventParameters *loadEventParameters();
		int load(EventParameters*);
		int loadPicks(EventParameters*);
		int loadAmplitudes(EventParameters*);
		int loadReadings(EventParameters*);
		int loadOrigins(EventParameters*);
		int loadFocalMechanisms(EventParameters*);
		int loadCatalogs(EventParameters*);
		int loadEvents(EventParameters*);
		int load(Pick*);
		int loadComments(Pick*);
		int load(Amplitude*);
		int loadComments(Amplitude*);
		int load(Reading*);
		int loadPickReferences(Reading*);
		int loadAmplitudeReferences(Reading*);
		int load(Origin*);
		int loadComments(Origin*);
		int loadCompositeTimes(Origin*);
		int loadArrivals(Origin*);
		int loadStationMagnitudes(Origin*);
		int loadMagnitudes(Origin*);
		int load(StationMagnitude*);
		int loadComments(StationMagnitude*);
		int load(Magnitude*);
		int loadComments(Magnitude*);
		int loadStationMagnitudeContributions(Magnitude*);
		int load(FocalMechanism*);
		int loadComments(FocalMechanism*);
		int loadMomentTensors(FocalMechanism*);
		int load(MomentTensor*);
		int loadComments(MomentTensor*);
		int loadDataUseds(MomentTensor*);
		int loadMomentTensorPhaseSettings(MomentTensor*);
		int loadMomentTensorStationContributions(MomentTensor*);
		int load(MomentTensorStationContribution*);
		int loadMomentTensorComponentContributions(MomentTensorStationContribution*);
		int load(Catalog*);
		int loadComments(Catalog*);
		int loadEvents(Catalog*);
		int load(Event*);
		int loadEventDescriptions(Event*);
		int loadComments(Event*);
		int loadOriginReferences(Event*);
		int loadFocalMechanismReferences(Event*);

		Config *loadConfig();
		int load(Config*);
		int loadParameterSets(Config*);
		int loadConfigModules(Config*);
		int load(ParameterSet*);
		int loadParameters(ParameterSet*);
		int loadComments(ParameterSet*);
		int load(Parameter*);
		int loadComments(Parameter*);
		int load(ConfigModule*);
		int loadConfigStations(ConfigModule*);
		int load(ConfigStation*);
		int loadSetups(ConfigStation*);

		QualityControl *loadQualityControl();
		int load(QualityControl*);
		int loadQCLogs(QualityControl*);
		int loadWaveformQualitys(QualityControl*);
		int loadOutages(QualityControl*);

		Inventory *loadInventory();
		int load(Inventory*);
		int loadStationGroups(Inventory*);
		int loadAuxDevices(Inventory*);
		int loadSensors(Inventory*);
		int loadDataloggers(Inventory*);
		int loadResponsePAZs(Inventory*);
		int loadResponseFIRs(Inventory*);
		int loadResponseIIRs(Inventory*);
		int loadResponsePolynomials(Inventory*);
		int loadResponseFAPs(Inventory*);
		int loadNetworks(Inventory*);
		int load(StationGroup*);
		int loadStationReferences(StationGroup*);
		int load(AuxDevice*);
		int loadAuxSources(AuxDevice*);
		int load(Sensor*);
		int loadSensorCalibrations(Sensor*);
		int load(Datalogger*);
		int loadDataloggerCalibrations(Datalogger*);
		int loadDecimations(Datalogger*);
		int load(Network*);
		int loadComments(Network*);
		int loadStations(Network*);
		int load(Station*);
		int loadComments(Station*);
		int loadSensorLocations(Station*);
		int load(SensorLocation*);
		int loadComments(SensorLocation*);
		int loadAuxStreams(SensorLocation*);
		int loadStreams(SensorLocation*);
		int load(Stream*);
		int loadComments(Stream*);

		Routing *loadRouting();
		int load(Routing*);
		int loadRoutes(Routing*);
		int loadAccesss(Routing*);
		int load(Route*);
		int loadRouteArclinks(Route*);
		int loadRouteSeedlinks(Route*);

		Journaling *loadJournaling();
		int load(Journaling*);
		int loadJournalEntrys(Journaling*);

		ArclinkLog *loadArclinkLog();
		int load(ArclinkLog*);
		int loadArclinkRequests(ArclinkLog*);
		int loadArclinkUsers(ArclinkLog*);
		int load(ArclinkRequest*);
		int loadArclinkStatusLines(ArclinkRequest*);
		int loadArclinkRequestLines(ArclinkRequest*);

		DataAvailability *loadDataAvailability();
		int load(DataAvailability*);
		int loadDataExtents(DataAvailability*);
		int load(DataExtent*);
		int loadDataSegments(DataExtent*);
		int loadDataAttributeExtents(DataExtent*);
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}


#endif
