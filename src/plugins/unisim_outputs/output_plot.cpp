/* Copyright (C) 2009-2011 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <QBrush>
#include <QMessageBox>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_symbol.h>
#include <usbase/dataset.h>
#include <usbase/file_locations.h>
#include <usbase/named_object.h>
#include <usbase/object_pool.h>
#include <usbase/output_data.h>
#include <usbase/output_variable.h>
#include <usbase/parameter.h>
#include <usbase/pull_variable_base.h>
#include <usengine/main_window_interface.h>
#include <usengine/plot_widget.h>
#include "output_plot.h"

namespace UniSim{

QList<QColor> OutputPlot::colors;

OutputPlot::OutputPlot(Identifier name, QObject *parent)
    : Output(name, parent), plotWidget(0)
{
    new Parameter<QString>("title", &title, QString(), this,
                           "Title of plot shown in window top bar");

    colors
    << QColor("#FF416E")
    << QColor("#3760D5")
    << QColor("#FFC741")
    << QColor("#34C1E8")
    << QColor("#F7FF41")
    << QColor("#A135D0")
    << QColor("#39DE52");
}

OutputPlot::~OutputPlot() {
    delete plotWidget;
}

void OutputPlot::initialize() {
    Output::initialize();

    const OutputResults &xs(xResults()), &ys(yResults());
    if (ys.size() == 0) {
        QString msg = "Output plot " + id().label() + " has no y-results";
        throw Exception(msg, this);
    }
    if (xs.size() == 0) {
        QString msg = "Output plot " + id().label() + " has no x-result";
        throw Exception(msg, this);
    }
    else if (xs.size() > 1) {
        QString msg = "Output plot " + id().label() + " has more than one x-result:";
        for (int i = 0; i < xs.size(); ++ i)
            msg += "\n" + xs.at(i)->id().key();
        throw Exception(msg, this);
    }

    mainWindow = objectPool()->find<MainWindowInterface*>("mainWindow");
    Q_ASSERT(mainWindow);
}

void OutputPlot::cleanup() {
    if (!isSummary()) {
        if (runNumber() == 1) createPlotWidget();
        showPlot();
    }
}

void OutputPlot::debrief() {
    if (isSummary()) {
        createPlotWidget();
        showPlot();
    }
}

void OutputPlot::showPlot() {
    fillPlotWidget();
    showPlotWidget();
    mainWindow->tile();
}

bool OutputPlot::emptyResults() const {
    return xResults().isEmpty() || yResults().isEmpty();
}

bool OutputPlot::emptyData() const {
    return xData().isEmpty() || yData().isEmpty();
}

void OutputPlot::createPlotWidget() {
    plotWidget = mainWindow->createPlotWidget(title);
    plotWidget->showLegend(true);
    //QwtPlotCanvas *canvas = plotWidget->plot()->canvas();
    //new QwtPlotZoomer(canvas);   crash!!�
}

void OutputPlot::fillPlotWidget() {
    Q_ASSERT(plotWidget);
    fillWithResults();
    fillWithData();
}

void OutputPlot::fillWithResults() {
    if (emptyResults()) return;

    OutputResults xv, yv;
    xv = xResults();
    yv = yResults();
    Q_ASSERT(xv.size() == 1);
    OutputResult *x = xv[0];

    QString yAxisTitle(" ");
    plotWidget->setXYtitles(x->id().label(), yAxisTitle);
    setYLabels();

    bool symbolsOnly = x->isOutputSummary();

    for (int i = 0; i < yv.size(); ++i) {
        OutputResult *y = yv[i];
        QwtPlotCurve *curve = new QwtPlotCurve(yLabels[i].label());
        bool showLegend = (runNumber() == 1);
        curve->setItemAttribute(QwtPlotItem::Legend, showLegend);
        plotWidget->addCurve(curve);

        QColor color = colors[i % colors.size()];
        QPen pen = QPen(color);
        pen.setWidth(2);

        if (symbolsOnly) {
            QBrush brush;
            QwtSymbol symbol(QwtSymbol::Ellipse, brush, pen, QSize(8,8));
            curve->setStyle(QwtPlotCurve::NoCurve);
            curve->setSymbol(symbol);
        }
        else {
            curve->setPen(pen);
        }


        int numPoints = x->history()->size();
        Q_ASSERT(numPoints == y->history()->size());
        curve->setData(x->history()->data(), y->history()->data(), numPoints);
    }
}

void OutputPlot::setYLabels() {
    if (!setYLabelsFromLabels() && yLabels.size() > 1)
        setYLabelsFromIds();
}

bool OutputPlot::setYLabelsFromLabels() {
    QList<NamedObject*> namedObjs;
    for (int i = 0; i < yResults().size(); ++i)
        namedObjs.append(yResults()[i]);

    bool areEqual;
    yLabels = getIds(namedObjs, &areEqual);
    return !areEqual;
}

QList<Identifier> OutputPlot::getIds(QList<NamedObject*> &objects, bool *areEqual) const{
    QList<Identifier> ids;
    *areEqual = true;
    Identifier prevId;
    for (int i = 0; i < objects.size(); ++i) {
        Identifier id = objects[i]->id();
        ids.append(id);
        *areEqual = *areEqual && (i==0 || id==prevId);
        prevId = id;
    }
    return ids;
}

void OutputPlot::setYLabelsFromIds() {
    QList<NamedObject*> ancestors;
    for (int i = 0; i < yResults().size(); ++i) {
        OutputVariable *var = dynamic_cast<OutputVariable*>(yResults()[i]);
        Q_ASSERT(var);
        NamedObject *ancestor = dynamic_cast<NamedObject*>(var->pullVariable()->parent());
        Q_ASSERT(ancestor);
        ancestors.append(ancestor);
    }

    bool areEqual(true), checkAgain(true);
    while (areEqual && checkAgain) {
        yLabels = getIds(ancestors, &areEqual);
        if (areEqual) {
            for (int i = 0; checkAgain && (i < ancestors.size()); ++i) {
                ancestors[i] = dynamic_cast<NamedObject*>(ancestors[i]->parent());
                checkAgain = ancestors[i];
            }
        }
    }
}

void OutputPlot::fillWithData() {
    if (emptyData()) return;

    QList<OutputData *> xv, yv;
    xv = xData();
    yv = yData();
    Q_ASSERT(xv.size() == 1);
    OutputData *x = xv[0];

    QString yAxisTitle(" ");
    plotWidget->setXYtitles(x->id().label(), yAxisTitle);
    plotWidget->showLegend(true);

    for (int i = 0; i < yv.size(); ++i) {
        OutputData *y = yv[i];
        QwtPlotCurve *curve = new QwtPlotCurve(y->id().label());
        plotWidget->addCurve(curve);

        QColor color = colors[i % colors.size()];
        QPen pen = QPen(color);
        pen.setWidth(2);

        QBrush brush;
        QwtSymbol symbol(QwtSymbol::Ellipse, brush, pen, QSize(8,8));
        curve->setStyle(QwtPlotCurve::NoCurve);
        curve->setSymbol(symbol);

        int numPoints = x->data()->size();
        Q_ASSERT(numPoints == y->data()->size());
        curve->setData(x->data()->data(), y->data()->data(), numPoints);
    }
}

void OutputPlot::showPlotWidget() {
    Q_ASSERT(plotWidget);
    plotWidget->show();
}

} //namespace

