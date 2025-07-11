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

#include <seiscomp/gui/map/standardlegend.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/map/canvas.h>

#include <QPainter>
#include <QFontMetrics>


namespace Seiscomp::Gui::Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(StandardLegend *legend) {
	if ( legend ) {
		legend->addItem(this);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(QPen p, QString l)
: pen(std::move(p)), label(std::move(l)) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(QPen p, QString l, int s)
: pen(std::move(p)), label(std::move(l)), size(s) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(QPen p, const QBrush &b, QString l)
: pen(std::move(p)), brush(b), label(std::move(l)) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(QPen p, const QBrush &b, QString l, int s)
: pen(std::move(p)), brush(b), label(std::move(l)), size(s) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(QPixmap sym, QString l)
: symbol(std::move(sym)), label(std::move(l)) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem::StandardLegendItem(QPixmap sym, QString l, int s)
: symbol(std::move(sym)), label(std::move(l)), size(s) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegendItem::draw(QPainter *painter, const QRect &symbolRect,
                              const QRect &textRect) {
	drawSymbol(painter, symbolRect);
	drawText(painter, textRect);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegendItem::drawSymbol(QPainter *painter, const QRect &rect) {
	// Draw symbol if available
	if ( !symbol.isNull() ) {
		painter->drawPixmap(rect, symbol);
		return;
	}

	// Limit pen width to 12.5% of rect width to ensure different line styles
	// can be distinguished.
	QPen p(pen);
	p.setWidth(qMax(1, qMin(rect.width() / 8, p.width())));
	painter->setPen(p);
	if ( brush != Qt::NoBrush ) {
		int x = rect.left();
		int y = rect.top();
		int w = rect.width();
		int h = rect.height();

		if ( size > 0 ) {
			int s = qMin(size, w);
			s = qMin(s, h);
			x += (w - s) / 2;
			y += (h - s) / 2;
			w = h = s;
		}

		painter->setBrush(brush);
		painter->drawRect(x, y, w, h);
	}
	else {
		int h = (rect.top() + rect.bottom() + p.width()) / 2;
		painter->drawLine(rect.left(), h, rect.right(), h);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegendItem::drawText(QPainter *painter, const QRect &rect) {
	painter->setPen(SCScheme.colors.legend.text);
	painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, label);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegend::StandardLegend(QObject *parent) : Legend(parent) {
	_orientation = Qt::Vertical;
	_maxColumns = -1;
	_layoutDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegend::~StandardLegend() {
	clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::addItem(StandardLegendItem *item) {
	_items.append(item);
	_layoutDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::clear() {
	qDeleteAll(_items);
	_items.clear();
	_layoutDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardLegendItem *StandardLegend::takeItem(int index) {
	StandardLegendItem *item = _items[index];
	_items.removeAt(index);
	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::setMaximumColumns(int columns) {
	if ( _maxColumns == columns ) {
		return;
	}

	_maxColumns = columns;
	_layoutDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::setOrientation(Qt::Orientation orientation) {
	if ( _orientation == orientation ) {
		return;
	}

	_orientation = orientation;
	_layoutDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::update() {
	if ( layer() ) {
		updateLayout(layer()->size());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::updateLayout(const QSize &size) {
	if ( !size.isValid() ) {
		return;
	}

	_columnWidth = 0;

	if ( _items.empty() ) {
		_layoutDirty = false;
		return;
	}

	QFontMetrics fm(font());
	int ch = size.height();
	int fontHeight = fm.height();
	int rows;
	int symbolSize = 0;

	for ( int i = 0; i < _items.count(); ++i ) {
		int itemWidth = fm.boundingRect(_items[i]->label).width();
		if ( itemWidth > _columnWidth ) {
			_columnWidth = itemWidth;
		}

		if ( _items[i]->size > symbolSize ) {
			symbolSize = _items[i]->size;
		}
	}

	symbolSize = qMax(symbolSize, fontHeight);
	_symbolSize = QSize(symbolSize, symbolSize);

	switch ( _orientation ) {
		case Qt::Vertical:
			_columns = 1;

			_size.setWidth((_columnWidth + _symbolSize.width() + fontHeight/2)*_columns + fontHeight + fontHeight/2*(_columns-1));
			_size.setHeight(qMax(((_items.count()+_columns-1)/_columns)*(_symbolSize.height() + fontHeight/2)+fontHeight/2, 0));

			if ( ch <= 0 ) {
				return;
			}

			while ( (_size.width() < size.width()) && (_size.height() > ch-30)
			     && ((_maxColumns <= 0) || (_columns < _maxColumns))
			     && (_columns < _items.size()) ) {
				++_columns;
				_size.setWidth((_columnWidth + _symbolSize.width() + fontHeight/2)*_columns + fontHeight + fontHeight/2*(_columns-1));
				_size.setHeight(qMax(((_items.count()+_columns-1)/_columns)*(_symbolSize.height() + fontHeight/2)+fontHeight/2, 0));
			}

			break;

		case Qt::Horizontal:
			_columns = _items.count();

			_size.setWidth((_columnWidth + _symbolSize.width() + fontHeight/2)*_columns + fontHeight + fontHeight/2*(_columns-1));
			_size.setHeight(qMax(((_items.count()+_columns-1)/_columns)*(_symbolSize.height() + fontHeight/2)+fontHeight/2, 0));

			if ( ch <= 0 ) {
				return;
			}

			rows = 1;

			while ( (_size.width() > size.width()-2*_margin) && (_size.height() < ch-30)
			     && ((_maxColumns <= 0) || (rows < _maxColumns))
			     && (_columns > 1) ) {
				++rows;
				_columns = (_items.count()+rows-1) / rows;
				_size.setWidth((_columnWidth + _symbolSize.width() + fontHeight/2)*_columns + fontHeight + fontHeight/2*(_columns-1));
				_size.setHeight(qMax(((_items.count()+_columns-1)/_columns)*(_symbolSize.height() + fontHeight/2)+fontHeight/2, 0));
			}

			break;

		default:
			break;
	}

	_layoutDirty = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::contextResizeEvent(const QSize &size) {
	updateLayout(size);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardLegend::draw(const QRect &rect, QPainter &painter) {
	if ( _layoutDirty && layer() ) {
		updateLayout(layer()->size());
	}

	if ( !_columnWidth ) {
		return;
	}

	QFontMetrics fm(font());
	int fontHeight = fm.height();

	painter.save();
	painter.setFont(font());

	int rowHeight = qMax(_symbolSize.height(), fontHeight);

	switch ( _orientation ) {
		case Qt::Vertical:
		{
			int idx = 0;
			int textWidth = rect.width() - fontHeight - fontHeight*3/2;

			QRect symbolRect(rect.left() + fontHeight/2, 0, 0, 0);
			QRect textRect(rect.left() + fontHeight + _symbolSize.width(), 0, 0, 0);

			for ( int c = 0; c < _columns; ++c ) {
				symbolRect.setTop(rect.top() + fontHeight/2);
				textRect.setTop(symbolRect.top());

				int itemsPerColumn = (_items.count() + _columns-1) / _columns;
				int cnt = _items.count() - itemsPerColumn * c;
				if ( cnt > itemsPerColumn ) {
					cnt = itemsPerColumn;
				}

				for ( int i = 0; i < cnt; ++i, ++idx ) {
					symbolRect.setSize(_symbolSize);
					textRect.setWidth(textWidth); textRect.setHeight(_symbolSize.height());

					_items[idx]->draw(&painter, symbolRect, textRect);

					symbolRect.setTop(symbolRect.top() + rowHeight + fontHeight/2);
					textRect.setTop(textRect.top() + rowHeight + fontHeight/2);
				}

				symbolRect.setLeft(symbolRect.left() + _columnWidth + fontHeight + _symbolSize.width());
				textRect.setLeft(textRect.left() + _columnWidth + fontHeight + _symbolSize.width());
			}
			break;
		}

		case Qt::Horizontal:
		{
			QRect symbolRect(rect.left() + fontHeight/2, 0, 0, 0);
			QRect textRect(rect.left() + fontHeight/2 + _symbolSize.width() + fontHeight/2, 0, 0, 0);

			for ( int c = 0; c < _columns; ++c ) {
				symbolRect.setTop(rect.top() + fontHeight/2);
				textRect.setTop(symbolRect.top());

				int itemsPerColumn = (_items.count() + _columns-1) / _columns;
				int cnt = _items.count() - itemsPerColumn * c;
				if ( cnt > itemsPerColumn ) {
					cnt = itemsPerColumn;
				}

				for ( int i = 0; i < cnt; ++i ) {
					int idx = i*_columns+c;
					symbolRect.setSize(_symbolSize);
					textRect.setWidth(_columnWidth); textRect.setHeight(_symbolSize.height());

					_items[idx]->draw(&painter, symbolRect, textRect);

					symbolRect.setTop(symbolRect.top() + rowHeight + fontHeight/2);
					textRect.setTop(textRect.top() + rowHeight + fontHeight/2);
				}

				symbolRect.setLeft(symbolRect.left() + fontHeight/2 + _symbolSize.width() + fontHeight/2 + _columnWidth);
				textRect.setLeft(textRect.left() + fontHeight/2 + _symbolSize.width() + fontHeight/2 + _columnWidth);
			}
			break;
		}

		default:
			break;
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Seiscomp::Gui::Map
