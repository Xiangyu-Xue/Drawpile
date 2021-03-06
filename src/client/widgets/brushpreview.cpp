/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2006-2016 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QDebug>

#include "core/point.h"
#include "core/layerstack.h"
#include "core/layer.h"
#include "core/shapes.h"
#include "core/floodfill.h"
#include "brushpreview.h"

#include <QPaintEvent>
#include <QPainter>
#include <QEvent>
#include <QMenu>

#ifndef DESIGNER_PLUGIN
namespace widgets {
#endif

BrushPreview::BrushPreview(QWidget *parent, Qt::WindowFlags f)
	: QFrame(parent,f), m_preview(nullptr), _sizepressure(false),
	_opacitypressure(false), _hardnesspressure(false), _smudgepressure(false),
	m_color(Qt::black), m_bg(Qt::white),
	_shape(Stroke), _fillTolerance(0), _fillExpansion(0), _underFill(false), _tranparentbg(false)
{
	setAttribute(Qt::WA_NoSystemBackground);
	setMinimumSize(32,32);

	_ctxmenu = new QMenu(this);

	_ctxmenu->addAction(tr("Change Foreground Color"), this, SIGNAL(requestColorChange()));
}

BrushPreview::~BrushPreview() {
	delete m_preview;
}

void BrushPreview::notifyBrushChange()
{
	m_needupdate = true;
	update();
	emit brushChanged(brush());
}

void BrushPreview::setPreviewShape(PreviewShape shape)
{
	_shape = shape;
	m_needupdate = true;
	update();
}

void BrushPreview::setTransparentBackground(bool transparent)
{
	_tranparentbg = transparent;
	m_needupdate = true;
	update();
}

void BrushPreview::setColor(const QColor& color)
{
	m_color = color;
	m_brush.setColor(color);

	// Decide background color
	const qreal lum = color.redF() * 0.216 + color.greenF() * 0.7152 + color.redF() * 0.0722;

	if(lum < 0.8) {
		m_bg = Qt::white;
	} else {
		m_bg = QColor(32, 32, 32);
	}

	notifyBrushChange();
}

void BrushPreview::setFloodFillTolerance(int tolerance)
{
	_fillTolerance = tolerance;
	m_needupdate = true;
	update();
}

void BrushPreview::setFloodFillExpansion(int expansion)
{
	_fillExpansion = expansion;
	m_needupdate = true;
	update();
}

void BrushPreview::setUnderFill(bool underfill)
{
	_underFill = underfill;
	m_needupdate = true;
	update();
}

void BrushPreview::resizeEvent(QResizeEvent *)
{ 
	m_needupdate = true;
}

void BrushPreview::changeEvent(QEvent *event)
{
	Q_UNUSED(event);
	m_needupdate = true;
	update();
}

void BrushPreview::paintEvent(QPaintEvent *event)
{
	if(m_needupdate)
		updatePreview();

	if((m_previewCache.isNull() || m_previewCache.size() != m_preview->size()) && m_preview->size().isValid())
		m_previewCache = QPixmap(m_preview->size());

	m_preview->paintChangedTiles(event->rect(), &m_previewCache);

	QPainter painter(this);
	painter.drawPixmap(event->rect(), m_previewCache, event->rect());
}

void BrushPreview::updatePreview()
{
	if(!m_preview) {
		m_preview = new paintcore::LayerStack;
		QSize size = contentsRect().size();
		m_preview->resize(0, size.width(), size.height(), 0);
		m_preview->createLayer(0, 0, QColor(0,0,0), false, false, QString());
	} else if(m_preview->width() != contentsRect().width() || m_preview->height() != contentsRect().height()) {
		m_preview->resize(0, contentsRect().width() - m_preview->width(), contentsRect().height() - m_preview->height(), 0);
	}

	QRectF previewRect(
		m_preview->width()/8,
		m_preview->height()/4,
		m_preview->width()-m_preview->width()/4,
		m_preview->height()-m_preview->height()/2
	);
	paintcore::PointVector pointvector;

	switch(_shape) {
	case Stroke: pointvector = paintcore::shapes::sampleStroke(previewRect); break;
	case Line:
		pointvector
			<< paintcore::Point(previewRect.left(), previewRect.top(), 1.0)
			<< paintcore::Point(previewRect.right(), previewRect.bottom(), 1.0);
		break;
	case Rectangle: pointvector = paintcore::shapes::rectangle(previewRect); break;
	case Ellipse: pointvector = paintcore::shapes::ellipse(previewRect); break;
	case FloodFill: pointvector = paintcore::shapes::sampleBlob(previewRect); break;
	}

	QColor bgcolor = m_bg;

	paintcore::Brush brush = m_brush;
	// Special handling for some blending modes
	// TODO this could be implemented in some less ad-hoc way
	if(brush.blendingMode() == 11) {
		// "behind" mode needs a transparent layer for anything to show up
		brush.setBlendingMode(paintcore::BlendMode::MODE_NORMAL);

	} else if(brush.blendingMode() == 12) {
		// Color-erase mode: use fg color as background
		bgcolor = m_color;
	}

	if(_shape == FloodFill) {
		brush.setColor(bgcolor);
	}

	paintcore::Layer *layer = m_preview->getLayerByIndex(0);
	layer->fillRect(QRect(0, 0, layer->width(), layer->height()), isTransparentBackground() ? QColor(Qt::transparent) : bgcolor, paintcore::BlendMode::MODE_REPLACE);

	paintcore::StrokeState ss(brush);
	for(int i=1;i<pointvector.size();++i)
		layer->drawLine(0, brush, pointvector[i-1], pointvector[i], ss);

	layer->mergeSublayer(0);

	if(_shape == FloodFill) {
		paintcore::FillResult fr = paintcore::floodfill(m_preview, previewRect.center().toPoint(), m_color, _fillTolerance, 0, false);
		if(_fillExpansion>0)
			fr = paintcore::expandFill(fr, _fillExpansion, m_color);
		layer->putImage(fr.x, fr.y, fr.image, _underFill ? paintcore::BlendMode::MODE_BEHIND : paintcore::BlendMode::MODE_NORMAL);
	}

	m_needupdate=false;
}

/**
 * @param brush brush to set
 */
void BrushPreview::setBrush(const paintcore::Brush& brush)
{
	m_brush = brush;
	notifyBrushChange();
}

/**
 * @param size brush size
 */
void BrushPreview::setSize(int size)
{
	m_brush.setSize(size);
	if(_sizepressure==false)
		m_brush.setSize2(size);
	notifyBrushChange();
}

/**
 * @param opacity brush opacity
 * @pre 0 <= opacity <= 100
 */
void BrushPreview::setOpacity(int opacity)
{
	const qreal o = opacity/100.0;
	m_brush.setOpacity(o);
	if(_opacitypressure==false)
		m_brush.setOpacity2(o);
	notifyBrushChange();
}

/**
 * @param hardness brush hardness
 * @pre 0 <= hardness <= 100
 */
void BrushPreview::setHardness(int hardness)
{
	const qreal h = hardness/100.0;
	m_brush.setHardness(h);
	if(_hardnesspressure==false)
		m_brush.setHardness2(h);
	notifyBrushChange();
}

/**
 * @param smudge color smudge pressure
 * @pre 0 <= smudge <= 100
 */
void BrushPreview::setSmudge(int smudge)
{
	const qreal s = smudge / 100.0;
	m_brush.setSmudge(s);
	if(_smudgepressure==false)
		m_brush.setSmudge2(s);
	notifyBrushChange();
}

/**
 * @param spacing dab spacing
 * @pre 0 <= spacing <= 100
 */
void BrushPreview::setSpacing(int spacing)
{
	m_brush.setSpacing(spacing);
	notifyBrushChange();
}

void BrushPreview::setSmudgeFrequency(int f)
{
	m_brush.setResmudge(f);
	notifyBrushChange();
}

void BrushPreview::setSizePressure(bool enable)
{
	_sizepressure = enable;
	if(enable)
		m_brush.setSize2(1);
	else
		m_brush.setSize2(m_brush.size1());
	notifyBrushChange();
}

void BrushPreview::setOpacityPressure(bool enable)
{
	_opacitypressure = enable;
	if(enable)
		m_brush.setOpacity2(0);
	else
		m_brush.setOpacity2(m_brush.opacity1());
	notifyBrushChange();
}

void BrushPreview::setHardnessPressure(bool enable)
{
	_hardnesspressure = enable;
	if(enable)
		m_brush.setHardness2(0);
	else
		m_brush.setHardness2(m_brush.hardness1());
	notifyBrushChange();
}

void BrushPreview::setSmudgePressure(bool enable)
{
	_smudgepressure = enable;
	if(enable)
		m_brush.setSmudge2(0);
	else
		m_brush.setSmudge2(m_brush.smudge1());
	notifyBrushChange();
}

void BrushPreview::setSubpixel(bool enable)
{
	m_brush.setSubpixel(enable);
	notifyBrushChange();
}

void BrushPreview::setBlendingMode(paintcore::BlendMode::Mode mode)
{
	m_brush.setBlendingMode(mode);
	notifyBrushChange();
}

void BrushPreview::setHardEdge(bool hard)
{
	if(hard) {
		_oldhardness1 = m_brush.hardness(0);
		_oldhardness2 = m_brush.hardness(1);
		m_brush.setHardness(1);
		m_brush.setHardness2(1);
		m_brush.setSubpixel(false);
	} else {
		m_brush.setHardness(_oldhardness1);
		m_brush.setHardness2(_oldhardness2);
		m_brush.setSubpixel(true);
	}
	notifyBrushChange();
}

void BrushPreview::setIncremental(bool incremental)
{
	m_brush.setIncremental(incremental);
	notifyBrushChange();
}

void BrushPreview::contextMenuEvent(QContextMenuEvent *e)
{
	_ctxmenu->popup(e->globalPos());
}

void BrushPreview::mouseDoubleClickEvent(QMouseEvent*)
{
	emit requestColorChange();
}

#ifndef DESIGNER_PLUGIN
}
#endif

