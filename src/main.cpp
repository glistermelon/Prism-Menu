// Prism Menu by Firee
// i am looking at you. make sure you credit me and this mod, i worked hard on it ;-;
// please do not judge my coding, ok thank you!!!
#include "CustomSettings.hpp"
#include <Geode/utils/file.hpp>
#include "PrismButton.hpp"
#include "hacks.hpp"
#include "Languages.hpp"
#include "Themes.hpp"
#include "PrismUI.hpp"
#include "Utils.hpp"
#include <Geode/Geode.hpp>
//#include <geode.custom-keybinds/include/Keybinds.hpp>
//using namespace keybinds;

#include <iomanip>
#include <string>
#include <locale>
#include <algorithm>

using namespace geode::prelude;

#include <Geode/modify/MenuLayer.hpp>

/*
for i in range(char2nr('A'), char2nr('Z'))
  call append(line("."), printf("{'%c', cocos2d::KEY_%c},", i, i))
endfor
*/

PrismButton* prismButton;

bool firstLoad = false;
// early load is amazing!

class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        /*auto username = static_cast<CCLabelBMFont*>(this->getChildByID("player-username"));
        if (username != nullptr) {
            auto normal = CCSprite::createWithSpriteFrameName("difficulty_02_btn_001.png");
            normal->setPosition({44,91});
            normal->setScale(2.0F);
            this->addChild(normal);
            username->setString("FIRE IN THE HOLE");
        }*/
        // lmao
        HackItem* posX = Hacks::getHack("Button Position X");
        HackItem* posY = Hacks::getHack("Button Position Y");
        auto mainMenu = static_cast<CCMenu*>(this->getChildByID("bottom-menu"));
        if (firstLoad) return true;
        firstLoad = true;
        log::info("Prism Menu loaded! Enjoy the mod.");
        #ifndef GEODE_IS_MACOS
        prismButton = PrismButton::create(CCScene::get());
        prismButton->setVisible(Hacks::isHackEnabled("Show Button"));
        prismButton->setID("prism-icon");
        SceneManager::get()->keepAcrossScenes(prismButton);
        #endif
        return true;
    }
};

// maybe this will fix the issue
$execute {
    SettingHackStruct val { matjson::Array() };
    Mod::get()->addCustomSetting<SettingHackValue>("values", val);
}

$on_mod(Loaded) {
    Hacks::processJSON(false);
    Themes::addToCurrentThemes();
    //auto getKeybindHack = Hacks::getHack("Open Menu Keybind");
    //char inputKeybind = 'C';
    //if (getKeybindHack != nullptr) inputKeybind = *(getKeybindHack->value.charValue);
    /*¶ Settings
    BindManager::get()->registerBindable({
        "prism-menu"_spr,
        "Toggle Prism Menu",
        "Keybind for toggling when to show Prism Menu's Mod Menu.",
        { Keybind::create(KEY_C, Modifier::None) },
        "Prism Menu/ModMenu"
    });

    new EventListener([=](InvokeBindEvent* event) {
        if (event->isDown()) {
            // do a backflip!
            showMenu = !showMenu;
        }
        return ListenerResult::Propagate;
    }, InvokeBindFilter(nullptr, "prism-menu"_spr));
    */
}

// remove when custom keybinds is fixed
#include <Geode/modify/CCKeyboardDispatcher.hpp>
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool arr) {
        if (down && (key == KEY_Tab)) {
            auto prismButton = typeinfo_cast<PrismButton*>(CCScene::get()->getChildByID("prism-icon"));
            HackItem* menuStyle = Hacks::getHack("Menu-Style");
            #ifdef NO_IMGUI 
            menuStyle->value.intValue = 1;
            #else
            if (prismButton == nullptr) return true;
            #endif
            if (menuStyle->value.intValue == 0) { // imgui
                prismButton->showImGuiMenu = !prismButton->showImGuiMenu;
            } else {
                auto prismUIExists = CCScene::get()->getChildByID("prism-menu");
                if (prismUIExists == nullptr) {
                    //PrismUI::create()->show();
                    auto p = PrismDynamicUI::create();
                    CCScene::get()->addChild(p);



                } else {
                    //static_cast<PrismUI*>(prismUIExists)->onClose(CCNode::create());
                }
            }
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);
    }
};

// sorry, not sorry. for some reason the allHacks vector isnt being initialized in the other .cpp file
// i completely wasted my time writing this whole patch script, and i kinda want android + mac support soooo

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

// showing the icon for android users lol
class $modify(PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        for (size_t i = 0; i < this->getChildrenCount(); i++) {
            auto obj = this->getChildren()->objectAtIndex(i);
            if (Utils::getNodeName(obj) == "cocos2d::CCMenu") {
                auto menu = static_cast<CCMenu*>(obj);
                auto button = PrismButton::createButton(this);
                button->setPositionX(-240);
                menu->addChild(button);
                break;
            }
        }
        
    }
};

// TODO: Check Cheat Indicator position
CircleButtonSprite* createCheatIndicator(bool isHacking) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto cheatIndicator = CircleButtonSprite::create(CCNode::create(), (isHacking) ? CircleBaseColor::Pink : CircleBaseColor::Green, CircleBaseSize::Tiny); //, CircleBaseColor::Geode
    cheatIndicator->setAnchorPoint({1,1});
    cheatIndicator->setPosition({28, winSize.height});
    cheatIndicator->setZOrder(1000);
    cheatIndicator->setTag(10420); // prevent PlayLayer from interfering
    return cheatIndicator;
}

#ifndef GEODE_IS_MACOS

#endif

class $modify(PlayLayer) {
    float previousPositionX = 0.0F;
    GameObject* antiCheatObject; // removing after lol
    CircleButtonSprite* cheatIndicator;
    CCLabelBMFont* accuracyLabel;
    bool previousTestMode;
    CCNode* prismNode;

    CCSprite* progressBar;
    CCLabelBMFont* percentLabel;
    CCLabelBMFont* attemptLabel;

    bool isCheating = false;
    
    // Anticheat Bypass
    bool initedDeath = false;

    // Noclip Accuracy
    int frame = 0;
    int death = 0;
    float previousPlayerX = 0.0F;
    float previousDeathX = 0.0F;

    // Anticheat Bypass, Noclip, No Spikes, No Solids
    void destroyPlayer(PlayerObject *p0, GameObject *p1) {
        //bool m_isTestMode = *reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + 0x413);
        //std::cout << m_isTestMode << std::endl;
        if (Hacks::isHackEnabled("Enable Patching")) return PlayLayer::destroyPlayer(p0, p1);
        bool noclipDisabled = !Hacks::isHackEnabled("No Solids") && !Hacks::isHackEnabled("Noclip");
        if ((noclipDisabled && !Hacks::isHackEnabled("No Spikes"))) return PlayLayer::destroyPlayer(p0, p1);
        
        if (Hacks::isHackEnabled("Anticheat Bypass")) {
            if (!m_fields->initedDeath) {
            #if !defined(GEODE_IS_ANDROID64) && !defined(GEODE_IS_MACOS)
            if (m_fields->antiCheatObject == nullptr && p1 != nullptr && (
                (p1->m_realXPosition == 0 && p1->m_realYPosition == p0->m_realYPosition) ||
                (p1->m_realXPosition == 0 && p1->m_realYPosition == p0->m_realYPosition) // todo, get player pos during PlayLayer::init
            )) { // thank you whamer100
                m_fields->antiCheatObject = p1;
                m_fields->initedDeath = true;
            } else if (!m_fields->initedDeath && m_fields->antiCheatObject == nullptr) {
                m_fields->antiCheatObject = p1;
                m_fields->initedDeath = true;
            }
            #else //sorry android64 users, someone has to implement fields for GameObject, because I get errors 
                if (!m_fields->initedDeath && m_fields->antiCheatObject == nullptr) {
                    m_fields->antiCheatObject = p1;
                    m_fields->initedDeath = true;
                }
                // bad coding
                /*
                for (int offset = 0x0; ; offset += 0x1) {
                    GameObject* val = reinterpret_cast<GameObject*>(reinterpret_cast<uintptr_t>(this) + offset);
                    if (val == p1) {
                        log::debug("Found Anticheat Object at offset 0x{}", offset);
                        m_fields->antiCheatObject = p1;
                        break;
                    }
                }
                //m_antiCheatObject = p1;
                m_fields->initedDeath = true;*/
            #endif
            }
            if (p1 == m_fields->antiCheatObject) { // nice AC robert
                return PlayLayer::destroyPlayer(p0, p1);
            }
            if (m_fields->accuracyLabel != nullptr) {
                m_fields->accuracyLabel->setColor({255,0,0});
            }
            if (m_player1 != nullptr) {
                if (m_player1->getPositionX() != m_fields->previousDeathX) {
                    m_fields->previousDeathX = m_player1->getPositionX();
                    m_fields->death += 1;
                }
            }
            if (Hacks::isHackEnabled("Suicide")) return PlayLayer::destroyPlayer(p0, p1);
        }
    }
    // Instant Complete, Hide Testmode
    bool init(GJGameLevel *p0, bool p1, bool p2) {
        if (!PlayLayer::init(p0,p1,p2)) return false;
        if (prismButton != nullptr && Hacks::isHackEnabled("Show Button")) prismButton->setVisible(false);
        if (Hacks::isHackEnabled("Instant Complete")) {
            PlayLayer::playEndAnimationToPos({2,2});
        }
        
        // 0xaa9
        int targetValue = true;
        /*for (int offset = 0x0; offset < 0xAAAA; offset += 0x1) {
            int val = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + offset);
            if (val == 0 || val == 1) {
            //std::cout << "Offset: 0x" << std::hex << offset << std::dec << ", Value: " << val << std::endl;
            }
            /\*if (val == targetValue) {
                std::cout << "Found target " << targetValue << " at offset 0x" << std::hex << offset << std::dec << std::endl;
                break;
            }*\/
        }*/
        #ifndef GEODE_IS_MACOS
        m_fields->previousTestMode = m_isTestMode;
        #endif
        if (Hacks::isHackEnabled("Level Edit")) {
            //m_fields->m_gameLevel->m_levelType = static_cast<GJLevelType>(2);
        }
        if (Hacks::isHackEnabled("Hide Testmode")) {
            for (size_t i = 0; i < this->getChildrenCount(); i++) { // i cant test if this works, bruh
                auto obj = this->getChildren()->objectAtIndex(i);
                if (Utils::getNodeName(obj) == "cocos2d::CCLabelBMFont") {
                    auto testModeLabel = static_cast<CCLabelBMFont*>(obj);
                    if (!strcmp(testModeLabel->getString(), "Testmode")) {
                        testModeLabel->setVisible(false);
                        break;
                    }
                }
            }
        }
        if (Hacks::isHackEnabled("Practice Music")) {
            GameStatsManager::sharedState()->toggleEnableItem(UnlockType::GJItem, 17, true);
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_fields->prismNode = CCNode::create();
        m_fields->prismNode->setTag(10420);
        m_fields->prismNode->setZOrder(1);
        m_fields->cheatIndicator = createCheatIndicator(false);
        m_fields->cheatIndicator->setVisible(Hacks::isHackEnabled("Cheat Indicator"));
        m_fields->accuracyLabel = CCLabelBMFont::create("100.00%", "bigFont.fnt");
        m_fields->accuracyLabel->setPosition({36, winSize.height - 35});
        m_fields->accuracyLabel->setScale(0.5F);
        m_fields->accuracyLabel->setOpacity(255 / 2);
        m_fields->accuracyLabel->setVisible(Hacks::isHackEnabled("Noclip Accuracy"));
        m_fields->accuracyLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
        m_fields->accuracyLabel->setZOrder(1000);
        m_fields->accuracyLabel->setTag(10420); // prevent PlayLayer from interfering
        m_fields->prismNode->addChild(m_fields->accuracyLabel);
        m_fields->prismNode->addChild(m_fields->cheatIndicator);
        this->addChild(m_fields->prismNode);
        return true;
    }
    void onQuit() {
        if (prismButton != nullptr && Hacks::isHackEnabled("Show Button")) prismButton->setVisible(true); // look at this
        m_fields->initedDeath = false;
        m_fields->frame = 0;
        m_fields->death = 0;
        PlayLayer::onQuit();
    }
    void resetLevel() {
        m_fields->frame = 0;
        m_fields->death = 0;
        PlayLayer::resetLevel();
    }
    void postUpdate(float p0) {
        PlayLayer::postUpdate(p0);
        if (m_player1 != nullptr) {
            if (Hacks::isHackEnabled("Suicide")) return PlayLayer::destroyPlayer(m_player1, nullptr);
            if (m_player1->getPositionX() != m_fields->previousPlayerX) {
                m_fields->previousPlayerX = m_player1->getPositionX();
                m_fields->frame += 1;
            }
        }
        if (m_fields->accuracyLabel != nullptr) {
            float accuracy = ((static_cast<float>(m_fields->frame - m_fields->death)) / static_cast<float>(m_fields->frame)) * 100; // for some reason this doesnt work on android, like it goes in the negatives
            m_fields->accuracyLabel->setString(fmt::format("{}%", Utils::setPrecision(accuracy, 2)).c_str());
            m_fields->accuracyLabel->setVisible(Hacks::isHackEnabled("Noclip Accuracy"));
            if (m_fields->frame % 4 == 0) { // quarter step
                m_fields->accuracyLabel->setColor({255,255,255});
            }
        }
#ifndef GEODE_IS_MACOS
        if (Hacks::isHackEnabled("Safe Mode") || Hacks::isAutoSafeModeActive()) {
            m_isTestMode = true;
        } else {
            m_isTestMode = m_fields->previousTestMode;
        }
#endif
        // whats the difference between m_fields and not using? i have no idea!
        if (Hacks::isCheating()) { // cheating
            if (!m_fields->isCheating) {
                m_fields->isCheating = true;
                if (Hacks::isHackEnabled("Cheat Indicator")) {
                    m_fields->cheatIndicator->removeFromParentAndCleanup(true);
                    m_fields->cheatIndicator = createCheatIndicator(true);
                    m_fields->prismNode->addChild(m_fields->cheatIndicator);
                }
            }
        } else { // not cheating
            if (m_fields->isCheating) {
                m_fields->isCheating = false;
                m_fields->cheatIndicator->removeFromParentAndCleanup(true);
                m_fields->cheatIndicator = createCheatIndicator(false);
                m_fields->prismNode->addChild(m_fields->cheatIndicator);
            }
        }
        if (Hacks::isHackEnabled("Instant Complete") && m_fields->frame < 5) {
            log::debug("CRIMINAL… criminal… criminal… criminal…");
            // funny message

            // Don't show if any form of safe mode is enabled, it gets VERY annoying otherwise
            if (!(Hacks::isHackEnabled("Safe Mode") || Hacks::isHackEnabled("Auto Safe Mode"))) FLAlertLayer::create(nullptr, "Cheater!", "Just a warning, you will be <cr>banned off leaderboards</c> if you use this on rated levels. Consider this your <cy>warning</c>.", "OK", nullptr)->show();
        }
        float attemptOpacity = Hacks::getHack("Attempt Opacity")->value.floatValue;
        //if (!Hacks::isHackEnabled("Hide Attempts") && attemptOpacity == 1.0F) return PlayLayer::postUpdate(p0);
        int currentPosition = Hacks::getHack("Progress Bar Position")->value.intValue;
        // stop dynamic_cast abuse
        auto node = typeinfo_cast<CCNode*>(this->getChildren()->objectAtIndex(0));
        if (node == nullptr) return; // never will happen (or will it)
        m_fields->cheatIndicator->setVisible(Hacks::isHackEnabled("Cheat Indicator"));
        if (m_fields->progressBar == nullptr || m_fields->percentLabel == nullptr || m_fields->attemptLabel == nullptr) {
            for (size_t i = 0; i < this->getChildrenCount(); i++) {
                auto obj = this->getChildren()->objectAtIndex(i);
                if (Utils::getNodeName(obj) == "cocos2d::CCLabelBMFont" && m_fields->percentLabel == nullptr) {
                    auto labelTest = static_cast<CCLabelBMFont*>(obj);
                    //if (strlen(labelTest->getString()) < 6) {
                    std::string labelStr = labelTest->getString();
                    if (labelStr.ends_with("%")) {
                        m_fields->percentLabel = labelTest;
                    }
                } else if (Utils::getNodeName(obj) == "cocos2d::CCSprite" && m_fields->progressBar == nullptr) {
                    m_fields->progressBar = static_cast<CCSprite*>(obj);
                }
            }
            for (size_t i = 0; i < node->getChildrenCount(); i++) {
                auto obj = node->getChildren()->objectAtIndex(i);
                if (Utils::getNodeName(obj) == "cocos2d::CCLayer") {
                    auto layer = static_cast<CCLayer*>(obj);
                    for (size_t y = 0; y < layer->getChildrenCount(); y++) {
                        auto obj2 = layer->getChildren()->objectAtIndex(y);
                        if (Utils::getNodeName(obj2) == "cocos2d::CCLabelBMFont") {
                            m_fields->attemptLabel = static_cast<CCLabelBMFont*>(obj2);
                            break;
                        }
                    }
                }
            }
        } else {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_fields->progressBar->setRotation(0.0F);
            switch (currentPosition) {
                case 0: // Top (312)
                    m_fields->progressBar->setPositionY(winSize.height - 8);
                    m_fields->percentLabel->setPositionY(winSize.height - 8);
                    break;
                case 1: // Bottom (10)
                    m_fields->progressBar->setPositionY(10);
                    m_fields->percentLabel->setPositionY(10);
                    break;
                case 2: // Left // 275
                    m_fields->progressBar->setPosition({ 10, winSize.height / 2 });
                    m_fields->percentLabel->setPosition({ 5, winSize.height - 45 }); // 275
                    m_fields->progressBar->setRotation(-90.0F);
                    break;
                case 3: // Right
                    m_fields->progressBar->setPosition({ winSize.width - 10, winSize.height / 2 });
                    m_fields->percentLabel->setPosition({ winSize.width - 40, winSize.height - 45 });
                    m_fields->progressBar->setRotation(-90.0F);
                    break;
            }
            m_fields->attemptLabel->setOpacity(attemptOpacity * 255);
        }
    }
    
    // Accurate Percentage
#ifndef GEODE_IS_MACOS
    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        if (Hacks::isHackEnabled("Accurate Percentage")) {
            if (m_fields->percentLabel == nullptr) return;
            std::stringstream percentStream;
            int numDigits = Hacks::getHack("Extra Percent Digits")->value.intValue;
            percentStream << std::fixed << std::setprecision(numDigits) << PlayLayer::getCurrentPercent() << "%";
            std::string percentStr = percentStream.str();
            m_fields->percentLabel->setString(percentStr.c_str());
        }
    }
#endif
    void levelComplete() {
        if (!(Hacks::isHackEnabled("Safe Mode") || Hacks::isAutoSafeModeActive()) || Hacks::isHackEnabled("Enable Patching")) return PlayLayer::levelComplete();
        PlayLayer::resetLevel(); // haha
    }
};

/*
class $modify(PlayLayer) {
    // Accurate Percentage

    /\*void levelComplete() {
        if (!Hacks::isHackEnabled("Safe Mode") || Hacks::isHackEnabled("Enable Patching")) return PlayLayer::levelComplete();
        PlayLayer::resetLevel(); // haha
    }
    void showNewBest(bool p0, int p1, int p2, bool p3, bool p4, bool p5) {
        if (!Hacks::isHackEnabled("Safe Mode")) return PlayLayer::showNewBest(p0, p1, p2, p3, p4, p5);
    }*\/ // WHY YOU HAVE DELAY
};
*/
