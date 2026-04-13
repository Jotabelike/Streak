#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

inline void addScrollBarToNode(CCNode* self, float offset, CCNode* addTo) {
    if (self->getUserObject("scrollbar"_spr)) return;

    CCNode* parent = self->getParent();
    if (!addTo) addTo = parent;
    if (!addTo) return;

    CCScrollLayerExt* scrollLayer = typeinfo_cast<CCScrollLayerExt*>(self);
    if (!scrollLayer) return;

    Scrollbar* scrollbar = Scrollbar::create(scrollLayer);
    if (!scrollbar) return;
    scrollbar->setID("scrollbar"_spr);
    scrollbar->setAnchorPoint(scrollLayer->getAnchorPoint());
    scrollbar->setZOrder(100);
    addTo->addChild(scrollbar);

    scrollbar->ignoreAnchorPointForPosition(scrollLayer->isIgnoreAnchorPointForPosition());
    float anchorX = scrollLayer->isIgnoreAnchorPointForPosition() ? 0 : scrollLayer->getAnchorPoint().x;
    scrollbar->setPosition(
        scrollLayer->getPositionX() - (scrollLayer->getContentWidth() * anchorX) + offset + scrollLayer->getContentWidth(),
        scrollLayer->getPositionY()
    );

    self->setUserObject("scrollbar"_spr, scrollbar);
}

inline void addScrollbar(CCNode* scrollLayer, float offset = 6.f, CCNode* addTo = nullptr) {
    if (addTo) {
        addScrollBarToNode(scrollLayer, offset, addTo);
    } else {
        queueInMainThread([self = Ref(scrollLayer), offset, addToRef = Ref(addTo)] {
            addScrollBarToNode(self, offset, addToRef);
        });
    }
}
