/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "positionernodeinstance.h"
#include <QQuickItem>

namespace QmlDesigner {
namespace Internal {

PositionerNodeInstance::PositionerNodeInstance(QQuickItem *item)
    : QuickItemNodeInstance(item)
{
}

bool PositionerNodeInstance::isPositioner() const
{
    return true;
}

bool PositionerNodeInstance::isLayoutable() const
{
    return true;
}

bool PositionerNodeInstance::isResizable() const
{
    return true;
}

PositionerNodeInstance::Pointer PositionerNodeInstance::create(QObject *object)
{
    QQuickItem *positioner = qobject_cast<QQuickItem*>(object);

    Q_ASSERT(positioner);

    Pointer instance(new PositionerNodeInstance(positioner));

    instance->setHasContent(anyItemHasContent(positioner));
    positioner->setFlag(QQuickItem::ItemHasContents, true);

    static_cast<QQmlParserStatus*>(positioner)->classBegin();

    instance->populateResetHashes();

    return instance;
}

void PositionerNodeInstance::refreshLayoutable()
{
    [[maybe_unused]] bool success = QMetaObject::invokeMethod(object(), "prePositioning");
    Q_ASSERT(success);
}

PropertyNameList PositionerNodeInstance::ignoredProperties() const
{
    static const PropertyNameList properties({"move", "add", "populate"});
    return properties;
}
} // namespace Internal
} // namespace QmlDesigner
