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


#include <seiscomp/datamodel/event.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/compat.h>
#include <seiscomp/gui/map/legend.h>

#include <QMenu>
#include <QMouseEvent>

#include "eventlayer.h"


namespace Seiscomp {
namespace Gui {
namespace {


void updateSymbol(Map::Canvas *canvas, OriginSymbol *symbol,
                  DataModel::Event *event, DataModel::Origin *org) {
	symbol->setLocation(org->latitude(), org->longitude());

	try {
		symbol->setDepth(org->depth());
	}
	catch ( ... ) {}

	DataModel::Magnitude *mag = DataModel::Magnitude::Find(event->preferredMagnitudeID());
	if ( mag != nullptr ) {
		try {
			symbol->setPreferredMagnitudeValue(mag->magnitude());
		}
		catch ( ... ) {
			symbol->setPreferredMagnitudeValue(0);
		}
	}
	else
		symbol->setPreferredMagnitudeValue(0);

	if ( canvas )
		symbol->calculateMapPosition(canvas);
}


}


#define HMARGIN (textHeight/2)
#define VMARGIN (textHeight/2)
#define SPACING (textHeight/2)
#define VSPACING (textHeight*3/4)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventLegend::EventLegend(QObject *parent) : Map::Legend(parent) {
	setArea(Qt::Alignment(Qt::AlignLeft | Qt::AlignBottom));

	Seiscomp::Gui::Gradient::iterator it = SCScheme.colors.originSymbol.depth.gradient.begin();
	QColor currentColor = it.value().first;
	++it;

	_depthItems.append(DepthItem(currentColor, StringWithWidth(QString("< %1").arg(it.key()),-1)));

	for ( ; it != SCScheme.colors.originSymbol.depth.gradient.end(); ++it ) {
		_depthItems.append(
			DepthItem(
				it.value().first,
				StringWithWidth(QString("%1").arg(it.key()),-1)
			)
		);
	}

	int fromMag = static_cast<int>(SCScheme.map.originSymbolMinMag < 0 ? ceil(SCScheme.map.originSymbolMinMag) : floor(SCScheme.map.originSymbolMinMag));
	for ( int i = fromMag; i <= fromMag + 7; ++i ) {
		_magItems.append(MagItem(Gui::OriginSymbol::getSize(i), StringWithWidth(QString::number(i), -1)));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventLegend::~EventLegend() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLegend::contextResizeEvent(const QSize &size) {
	// Compute size
	QFont f(font());
	QFontMetrics fm(f);

	int textHeight = fm.height();

	int width = 0;
	int height = textHeight*3 + 3*VSPACING;

	QString depthHeader = tr("Depth in km");
	QString magHeader = tr("Magnitudes");

	width = qMax(width, fm.boundingRect(depthHeader).width());
	width = qMax(width, fm.boundingRect(magHeader).width());

	_depthWidth = 0;

	int cnt = _depthItems.count();
	for ( int i = 0; i < cnt; ++i ) {
		_depthItems[i].second.second = QT_FM_WIDTH(fm, _depthItems[i].second.first);
		_depthWidth += _depthItems[i].second.second + SPACING/2 + textHeight;
	}

	_depthWidth += SPACING * (cnt-1);

	width = qMax(width, _depthWidth);

	_magWidth = 0;
	_magHeight = 0;

	cnt = _magItems.count();
	for ( int i = 0; i < cnt; ++i ) {
		_magItems[i].second.second = QT_FM_WIDTH(fm, _magItems[i].second.first);
		_magWidth += _magItems[i].first + SPACING/2 + _magItems[i].second.second;
		_magHeight = qMax(_magHeight, _magItems[i].first);
	}

	_magWidth += SPACING * (cnt-1);

	width = qMax(width, _magWidth);
	height += _magHeight;

	width += HMARGIN*2;
	height += VMARGIN*2;

	_size.setWidth(width);
	_size.setHeight(height);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLegend::draw(const QRect &rect, QPainter &p) {
	p.save();

	QFont f(font());
	QFont bold(f);
	QFontMetrics fm(f);
	bold.setBold(true);

	int textHeight = fm.height();

	QString depthHeader = tr("Depth in km");
	QString magHeader = tr("Magnitudes");

	int width = _size.width();

	int x = rect.left();
	int y = rect.top();

	p.setRenderHint(QPainter::Antialiasing, false);

	y += VMARGIN;

	p.setFont(bold);
	p.setPen(SCScheme.colors.legend.headerText);
	p.drawText(QRect(x, y, width, textHeight),
	           Qt::AlignHCenter | Qt::AlignTop, depthHeader);

	y += textHeight+VSPACING;

	// Render depth items
	int cnt = _depthItems.count();

	qreal additionalItemSpacing = 0;
	if ( cnt > 1 )
		additionalItemSpacing = qreal(width - HMARGIN*2 - _depthWidth) / qreal(cnt-1);

	p.setPen(SCScheme.colors.legend.text);
	p.setFont(f);

	qreal fX = x + HMARGIN;
	for ( int i = 0; i < cnt; ++i ) {
		p.setBrush(_depthItems[i].first);
		p.drawRect((int)fX, y, textHeight, textHeight);
		fX += textHeight + SPACING/2;

		p.drawText(QRect((int)fX, y, _depthItems[i].second.second, textHeight),
		           Qt::AlignLeft | Qt::AlignTop, _depthItems[i].second.first);
		fX += _depthItems[i].second.second + SPACING + additionalItemSpacing;
	}

	y += textHeight + VSPACING;

	// Render magnitude items

	p.setFont(bold);
	p.setPen(SCScheme.colors.legend.headerText);
	p.drawText(QRect(x, y, width, textHeight),
	           Qt::AlignHCenter | Qt::AlignTop, magHeader);

	y += textHeight + VSPACING;

	cnt = _magItems.count();

	additionalItemSpacing = 0;
	if ( cnt > 1 )
		additionalItemSpacing = qreal(width - HMARGIN*2 - _magWidth) / qreal(cnt-1);

	p.setPen(QPen(SCScheme.colors.legend.text,2));
	p.setBrush(Qt::gray);
	p.setFont(f);

	fX = x + HMARGIN;

	p.setRenderHint(QPainter::Antialiasing, true);

	for ( int i = 0; i < cnt; ++i ) {
		p.drawEllipse((int)fX, y + (_magHeight-_magItems[i].first)/2, _magItems[i].first, _magItems[i].first);
		fX += _magItems[i].first + SPACING/2;

		p.drawText(QRect((int)fX, y, _magItems[i].second.second, _magHeight),
		           Qt::AlignLeft | Qt::AlignVCenter, _magItems[i].second.first);
		fX += _magItems[i].second.second + SPACING + additionalItemSpacing;
	}

	p.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventLayer::EventLayer(QObject* parent) : Map::Layer(parent) {
	setName("events");
	setVisible(false);
	_hoverChanged = false;

	EventLegend *legend = new EventLegend(this);
	legend->setTitle(tr("Event symbols"));
	addLegend(legend);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventLayer::~EventLayer() {
	// Delete all symbols
	for ( auto &symbol : _eventSymbols ) {
		delete symbol;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::draw(const Map::Canvas *canvas, QPainter &p) {
	for ( auto &symbol : _eventSymbols ) {
		if ( symbol->isClipped() || !symbol->isVisible() ) {
			continue;
		}
		symbol->draw(canvas, p);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::calculateMapPosition(const Map::Canvas *canvas) {
	// Update position of all symbols
	for ( auto &symbol : _eventSymbols ) {
		symbol->calculateMapPosition(canvas);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventLayer::isInside(const QMouseEvent *event, const QPointF &geoPos) {
	int x = event->pos().x();
	int y = event->pos().y();
	auto it = _eventSymbols.end();

	while ( it != _eventSymbols.begin() ) {
		--it;
		if ( it.value()->isClipped() || !it.value()->isVisible() ) {
			continue;
		}
		if ( it.value()->isInside(x, y) ) {
			_hoverChanged = _hoverId != it.key();
			if ( _hoverChanged ) {
				_hoverId = it.key();
			}
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::handleEnterEvent() {
	//
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::handleLeaveEvent() {
	_hoverChanged = !_hoverId.empty();
	_hoverId = std::string();
	emit eventHovered(_hoverId);
	_hoverChanged = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventLayer::filterMouseMoveEvent(QMouseEvent *event, const QPointF &geoPos) {
	if ( _hoverChanged ) {
		emit eventHovered(_hoverId);
		_hoverChanged = false;
	}
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventLayer::filterMouseDoubleClickEvent(QMouseEvent *event, const QPointF &geoPos) {
	if ( _hoverId.empty() )
		return false;

	if ( event->button() == Qt::LeftButton ) {
		emit eventSelected(_hoverId);
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::clear() {
	// Delete all symbols
	for ( auto &symbol : _eventSymbols ) {
		delete symbol;
	}

	_eventSymbols.clear();
	_hoverChanged = false;
	_hoverId = std::string();
	emit updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::addEvent(Seiscomp::DataModel::Event *e, bool) {
	auto it = _eventSymbols.find(e->publicID());

	DataModel::Origin *org = DataModel::Origin::Find(e->preferredOriginID());
	if ( org != nullptr ) {
		OriginSymbol *symbol;

		if ( it == _eventSymbols.end() )
			symbol = new OriginSymbol();
		else
			symbol = it.value();

		updateSymbol(canvas(), symbol, e, org);
		_eventSymbols[e->publicID()] = symbol;

		// Create origin symbol and register it
		emit updateRequested();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::updateEvent(Seiscomp::DataModel::Event *e) {
	auto it = _eventSymbols.find(e->publicID());
	if ( it == _eventSymbols.end() ) {
		return;
	}

	DataModel::Origin *org = DataModel::Origin::Find(e->preferredOriginID());
	if ( org != nullptr ) {
		updateSymbol(canvas(), it.value(), e, org);
		emit updateRequested();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventLayer::removeEvent(Seiscomp::DataModel::Event *e) {
	auto it = _eventSymbols.find(e->publicID());
	if ( it == _eventSymbols.end() ) {
		return;
	}
	delete it.value();
	_eventSymbols.erase(it);
	emit updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
