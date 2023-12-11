/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2001-2023 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    GUIPostDrawing.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 22
///
// Operations that must be applied after drawGL()
/****************************************************************************/
#include <config.h>
#include <algorithm>

#include <utils/shapes/Shape.h>

#include "GUIPostDrawing.h"


GUIPostDrawing::GUIPostDrawing() {}


void
GUIPostDrawing::clearElements() {
    // reset recompute boundaries
    recomputeBoundaries = GLO_NETWORK;
    // clear objects under cursor
    myElementsUnderCursor.clear();
    // reset marked elements
    markedEdge = nullptr;
    markedLane = nullptr;
    markedTAZ = nullptr;
    markedRoute = nullptr;
    markedFirstGeometryPoint = nullptr;
    markedSecondGeometryPoint = nullptr;
}


bool
GUIPostDrawing::isElementUnderCursor(const GUIGlObject* GLObject) const {
    // avoid to insert duplicated elements
    for (auto &elementLayer : myElementsUnderCursor) {
        for (auto &element : elementLayer.second) {
            if (element.first == GLObject) {
                return true;
            }
        }
    }
    return false;
}


bool
GUIPostDrawing::isGeometryPointUnderCursor(const GUIGlObject* GLObject, const int index) const {
    // avoid to insert duplicated elements
    for (auto &elementLayer : myElementsUnderCursor) {
        for (auto &element : elementLayer.second) {
            if (element.first == GLObject) {
                return std::find(element.second.begin(), element.second.end(), index) != element.second.end();
            }
        }
    }
    return false;
}


bool
GUIPostDrawing::positionWithinCircle(const GUIGlObject* GLObject, const Position &pos, const Position &center, const double radius) {
    if (pos.distanceSquaredTo2D(center) <= (radius * radius)) {
        return addElementUnderCursor(GLObject);
    } else {
        return false;
    }
}


bool
GUIPostDrawing::positionWithinGeometryPoint(const GUIGlObject* GLObject, const Position &pos, const int index, const Position &center, const double radius) {
    if (pos.distanceSquaredTo2D(center) <= (radius * radius)) {
        return addGeometryPointUnderCursor(GLObject, index);
    } else {
        return false;
    }
}


bool
GUIPostDrawing::positionWithinShape(const GUIGlObject* GLObject, const Position &pos, const PositionVector &shape) {
    if (shape.around(pos)) {
        return addElementUnderCursor(GLObject);
    } else {
        return false;
    }
}


const GUIPostDrawing::GLObjectsSortedContainer&
GUIPostDrawing::getElementsUnderCursor() const {
    return myElementsUnderCursor;
}


const GUIPostDrawing::GeometryPointsContainer&
GUIPostDrawing::getGeometryPoints(const GUIGlObject* GLObject) const {
    // avoid to insert duplicated elements
    for (auto &elementLayer : myElementsUnderCursor) {
        for (auto &element : elementLayer.second) {
            if (element.first == GLObject) {
                return element.second;
            }
        }
    }
    return myEmptyGeometryPoints;
}


void
GUIPostDrawing::updateFrontElement(const GUIGlObject* GLObject) {
    GLObjectContainer frontElement(nullptr, {});
    // extract element
    for (auto &elementLayer : myElementsUnderCursor) {
        auto it = elementLayer.second.begin();
        while (it != elementLayer.second.end()) {
            if (it->first == GLObject) {
                // copy element to front element
                frontElement.first = it->first;
                frontElement.second = it->second;
                // remove element from myElementsUnderCursor
                it = elementLayer.second.erase(it);
            } else {
                it++;
            }
        }
    }
    // add element again wit a new layer
    if (frontElement.first) {
        myElementsUnderCursor[(double)GLO_FRONTELEMENT].push_back(frontElement);
    }
}


bool
GUIPostDrawing::addElementUnderCursor(const GUIGlObject* GLObject) {
    // avoid to insert duplicated elements
    if (isElementUnderCursor(GLObject)) {
        return false;
    } else {
        // check if this is an element with an associated layer
        const auto layer = dynamic_cast<const Shape*>(GLObject);
        if (layer) {
            auto &layerContainer = myElementsUnderCursor[layer->getShapeLayer() * -1];
            layerContainer.insert(layerContainer.begin(), std::make_pair(GLObject, myEmptyGeometryPoints));
        } else {
            auto &layerContainer = myElementsUnderCursor[GLObject->getType() * -1];
            layerContainer.insert(layerContainer.begin(), std::make_pair(GLObject, myEmptyGeometryPoints));
        }
        return true;
    }
}


bool
GUIPostDrawing::addGeometryPointUnderCursor(const GUIGlObject* GLObject, const int newIndex) {
    // avoid to insert duplicated elements
    for (auto &elementLayer : myElementsUnderCursor) {
        for (auto &element : elementLayer.second) {
            if (element.first == GLObject) {
                // avoid double points
                for (auto &index : element.second) {
                    if (index == newIndex) {
                        return false;
                    }
                }
                // add new index
                element.second.push_back(newIndex);
                return true;
            }
        }
    }
    // no element found then add it
    const auto layer = dynamic_cast<const Shape*>(GLObject);
    if (layer) {
        auto &layerContainer = myElementsUnderCursor[layer->getShapeLayer() * -1];
        auto it = layerContainer.insert(layerContainer.begin(), std::make_pair(GLObject, myEmptyGeometryPoints));
        it->second.push_back(newIndex);
    } else {
        auto &layerContainer = myElementsUnderCursor[GLObject->getType() * -1];
        auto it = layerContainer.insert(layerContainer.begin(), std::make_pair(GLObject, myEmptyGeometryPoints));
        it->second.push_back(newIndex);
    }
    return true;
}

/****************************************************************************/