/******************************************************************************
  This source file is part of the Avogadro project.
  This source code is released under the 3-Clause BSD License, (see "LICENSE").
******************************************************************************/

#include "crystalscene.h"

#include <avogadro/core/array.h>
#include <avogadro/core/unitcell.h>
#include <avogadro/qtgui/colorbutton.h>
#include <avogadro/qtgui/molecule.h>
#include <avogadro/rendering/geometrynode.h>
#include <avogadro/rendering/groupnode.h>
#include <avogadro/rendering/linestripgeometry.h>

#include <QtCore/QSettings>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QVBoxLayout>

#include <QDebug>

namespace Avogadro::QtPlugins {

using Core::Array;
using Core::UnitCell;
using Rendering::GeometryNode;
using Rendering::GroupNode;
using Rendering::LineStripGeometry;

CrystalScene::CrystalScene(QObject* p)
  : ScenePlugin(p), m_setupWidget(nullptr)
{
  m_layerManager = QtGui::PluginLayerManager(m_name);

  QSettings settings;
  m_lineWidth = settings.value("crystal/lineWidth", 2.0).toDouble();

  auto color =
    settings.value("crystal/color", QColor(Qt::white)).value<QColor>();
  m_color[0] = static_cast<unsigned char>(color.red());
  m_color[1] = static_cast<unsigned char>(color.green());
  m_color[2] = static_cast<unsigned char>(color.blue());
}

CrystalScene::~CrystalScene() {}

void CrystalScene::process(const QtGui::Molecule& molecule, GroupNode& node)
{
  if (const UnitCell* cell = molecule.unitCell()) {
    auto* geometry = new GeometryNode;
    node.addChild(geometry);
    auto* lines = new LineStripGeometry;
    geometry->addDrawable(lines);

    lines->setColor(m_color);
    float width = m_lineWidth;

    Vector3f a = cell->aVector().cast<float>();
    Vector3f b = cell->bVector().cast<float>();
    Vector3f c = cell->cVector().cast<float>();

    Vector3f vertex(Vector3f::Zero());

    Array<Vector3f> strip;
    strip.reserve(5);
    strip.push_back(vertex);
    strip.push_back(vertex += a);
    strip.push_back(vertex += b);
    strip.push_back(vertex -= a);
    strip.push_back(vertex -= b);
    lines->addLineStrip(strip, width);

    for (auto & it : strip) {
      it += c;
    }
    lines->addLineStrip(strip, width);

    strip.resize(2);
    strip[0] = Vector3f::Zero();
    strip[1] = c;
    lines->addLineStrip(strip, width);

    strip[0] += a;
    strip[1] += a;
    lines->addLineStrip(strip, width);

    strip[0] += b;
    strip[1] += b;
    lines->addLineStrip(strip, width);

    strip[0] -= a;
    strip[1] -= a;
    lines->addLineStrip(strip, width);
  }
}

void CrystalScene::setLineWidth(double width)
{
  m_lineWidth = width;
  emit drawablesChanged();

  QSettings settings;
  settings.setValue("crystal/lineWidth", width);
}

void CrystalScene::setColor(const QColor& color)
{
  m_color[0] = static_cast<unsigned char>(color.red());
  m_color[1] = static_cast<unsigned char>(color.green());
  m_color[2] = static_cast<unsigned char>(color.blue());

  emit drawablesChanged();

  QSettings settings;
  settings.setValue("crystal/color", color);
}

QWidget* CrystalScene::setupWidget()
{
  if (!m_setupWidget) {
    m_setupWidget = new QWidget(qobject_cast<QWidget*>(parent()));
    auto* v = new QVBoxLayout;

    // line width
    auto* spin = new QDoubleSpinBox;
    spin->setRange(0.5, 5.0);
    spin->setSingleStep(0.25);
    spin->setDecimals(2);
    spin->setValue(m_lineWidth);
    connect(spin, SIGNAL(valueChanged(double)), SLOT(setLineWidth(double)));
    auto* form = new QFormLayout;
    form->addRow(tr("Line width:"), spin);

    auto* color = new QtGui::ColorButton;
    connect(color, SIGNAL(colorChanged(const QColor&)),
            SLOT(setColor(const QColor&)));
    form->addRow(tr("Line color:"), color);

    v->addLayout(form);

    v->addStretch(1);
    m_setupWidget->setLayout(v);
  }
  return m_setupWidget;
}

} // namespace Avogadro
