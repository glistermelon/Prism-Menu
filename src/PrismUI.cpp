#include "PrismUI.hpp"
#include "GatoSim.hpp"
#include "Themes.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/TextArea.hpp>
#include "Dropdown.h"
#include "Utils.hpp"


int currentMenuIndexGD = 0;

matjson::Array currentThemeApplied;

int calcLimit(int n) {
    return static_cast<int>(std::floor(std::log10(n)) + 1);
}

int currentI = 0;
void PrismUI::ButtonState(int id, bool activated) {
    auto buttonBG = static_cast<cocos2d::extension::CCScale9Sprite*>(m_buttonMenu->getChildByID(fmt::format("prism-nav-bg-{}", id)));
    auto button = static_cast<CCMenuItemSpriteExtra*>(m_buttonMenu->getChildByID(fmt::format("prism-nav-btn-{}", id)));
    if (activated) {
        button->setColor({255,255,255});
        Themes::RGBAToCC(GetTheme()["ButtonActive"], buttonBG);
        currentMenuIndexGD = id;
    } else {
        Themes::RGBAToCC(GetTheme()["Button"], buttonBG);
        Themes::RGBAToCC(GetTheme()["Text"], button);
    }
}

cocos2d::extension::CCScale9Sprite* PrismUIButton::createCheckbox(bool check) {
    auto box = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 0.0f, 0.0f });
    box->setScale(0.3F);
    if (check) {
        auto checkmark = CCSprite::create("checkmark.png"_spr);
        checkmark->setScale(0.3F);
        checkmark->setAnchorPoint({0,0});
        checkmark->setPosition({6,15});
        box->addChild(checkmark);
        Themes::RGBAToCC(PrismUI::GetTheme()["CheckMark"], checkmark);
    }
    Themes::RGBAToCC(PrismUI::GetTheme()["FrameBg"], box);
    return box;
}

float calculateYPosition(float x) { // love floating points
    // Formula: Y = mx + b
    float m = 85;
    float result = -m + (((10 - x) * m) / 2);
    return result;
}

int getYPosBasedOnCategory(int length) { // someone give me a proper math formula ok thanks
    /*switch (currentMenuIndexGD) {
        case 0: // Global 
            return 40;
        case 1: // Player
            return 70;
        case 2: // Bypass
            return -100;
        case 3: // Creator
            return -100;
        case 4: // Misc
            return -100;
        case 5: // Settings
            return 115;
        default: return 0;
    }*/
    if (length <= 8) return -100;
    if (currentMenuIndexGD == 5) length += 1;
    return (30 * length) - 350;
}
float getContentSizeBasedOnCategory(int length) { // someone give me a proper math formula ok thanks
    // 8
    /*
    switch (currentMenuIndexGD) {
        case 0: // Global 
            return 370; // 260
        case 1: // Player
            return 400;
        case 2: // Bypass
            return 230; // 230
        case 3: // Creator
            return 230; // 230
        case 4: // Misc
            return 230; // 230
        case 5: // Settings
            return 450; // 400
        default: return 320;
    }*/
    if (length <= 8) return 230;
    if (currentMenuIndexGD == 5) length += 2;
    return (29 * length) - 7;
}

float getSliderValue(float current, float min, float max, bool inverse) {
    return (inverse) ? (current * (max - min) + min) : (current - min) / (max - min);
}

// since SimpleTextArea doesnt allow limitLabelWidth
float calculateScale(const std::string& input, int charsBeforeShrink, float minScale, float maxScale) {
    int inputLength = input.length();
    if (inputLength > charsBeforeShrink) {
        float scale = minScale + (maxScale - minScale) * (static_cast<float>(inputLength - charsBeforeShrink) / inputLength);
        return scale;
    } else {
        return minScale;
    }
}

bool PrismUIButton::init(HackItem* hack) {
    menu = CCMenu::create();
    const auto& obj = hack->data;
    std::string name = hack->name;
    std::string desc = hack->desc;
    //auto opcodes = obj.get<matjson::Array>("opcodes");
    this->m_hack = hack;
    auto label = CCLabelBMFont::create(currentLanguage->name(name).c_str(), "PrismMenu.fnt"_spr);
    Themes::RGBAToCC(PrismUI::GetTheme()["Text"], label);
    label->setAnchorPoint({0.0F, 0.5F});
    label->limitLabelWidth(150, 0.5F, .2F);
    label->setPositionX(20);
    menu->addChild(label);
    //Themes::RGBAToCC(PrismUI::GetTheme()["Text"], reinterpret_cast<CCNodeRGBA*>(label)); // prrrobably not a good idea
    if (hack->value.type == ValueType::Bool) {
        auto checkbox = CCMenuItemToggler::create(
            createCheckbox(hack->value.boolValue), createCheckbox(!hack->value.boolValue),
            this,
            menu_selector(PrismUIButton::onBoolBtn)
        );
        //checkbox->setUserData(reinterpret_cast<void*>(hack));
        menu->addChild(checkbox);
        if (obj.contains("win")) {
            #ifndef GEODE_IS_WINDOWS
            checkbox->setEnabled(false);
            #endif
        } else if (name == "Enable Patching") {
            checkbox->setEnabled(false);
        }
    } else if (hack->value.type == ValueType::Int && hack->type != "dropdown" && !name.starts_with("Button Position")) {
        auto min = obj.get<int>("min");
        auto max = obj.get<int>("max");
        label->limitLabelWidth(120, 0.5F, .1F);
        // TODO: add + and - buttons because according to some, android keyboard is BAD!
        m_input = TextInput::create(150.f, "...", "PrismMenu.fnt"_spr);
        m_input->setString(std::to_string(hack->value.intValue));
        m_input->setFilter("0123456789");
        m_input->setPositionX(37);
        
        auto incBtnSpr = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
        incBtnSpr->setContentSize({ 50.0f, 50.0f });
        Themes::RGBAToCC(PrismUI::GetTheme()["Button"], incBtnSpr);
        incBtnSpr->setScale(.4F);

        auto decBtnSpr = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
        decBtnSpr->setContentSize({ 50.0f, 50.0f });
        Themes::RGBAToCC(PrismUI::GetTheme()["Button"], decBtnSpr);
        decBtnSpr->setScale(.4F);

        auto incBtnlabel = CCLabelBMFont::create("+", "PrismMenu.fnt"_spr);
        Themes::RGBAToCC(PrismUI::GetTheme()["Text"], incBtnlabel);
        incBtnlabel->setPosition({ incBtnSpr->getContentSize().width / 2, incBtnSpr->getContentSize().height / 2 });
        auto decBtnlabel = CCLabelBMFont::create("-", "PrismMenu.fnt"_spr);
        Themes::RGBAToCC(PrismUI::GetTheme()["Text"], decBtnlabel);
        decBtnlabel->setPosition({ incBtnSpr->getContentSize().width / 2, incBtnSpr->getContentSize().height / 2 });

        incBtnSpr->addChild(incBtnlabel);
        decBtnSpr->addChild(decBtnlabel);

        auto incBtn = CCMenuItemSpriteExtra::create(
            incBtnSpr,
            this,
            menu_selector(PrismUIButton::onIncBtn)
        );
        incBtn->setPositionX(124);
        menu->addChild(incBtn);
        auto decBtn = CCMenuItemSpriteExtra::create(
            decBtnSpr,
            this,
            menu_selector(PrismUIButton::onDecBtn)
        );
        decBtn->setPositionX(100);
        menu->addChild(decBtn);

        label->setPositionX(140);
    } else if (hack->value.type == ValueType::Float || name.starts_with("Button Position")) {
        auto value = (name.starts_with("Button Position")) ? hack->value.intValue : hack->value.floatValue;

        label->limitLabelWidth(64, 0.5F, .1F);
        auto min = obj.get<int>("min");
        auto max = obj.get<int>("max");
        m_input = TextInput::create(100.f, "...", "PrismMenu.fnt"_spr);
        m_input->setString(
            name.starts_with("Button Position") ? std::to_string(value) : Utils::setPrecision(value, 3)
        );
        m_input->setFilter("0123456789.");
        m_input->setPositionX(21);
        label->setPositionX(190);
        m_slider = Slider::create(this, menu_selector(PrismUIButton::onFloatBtn), .6f);
        m_slider->setPositionX(122);
        m_slider->setValue(getSliderValue(value, min, max, false));
        menu->addChild(m_slider);
    } else if (hack->type == "dropdown" || hack->value.type == ValueType::Custom) {
        auto type = obj.get<std::string>("type");
        if (type == "button") {
            //const char* caption, int width, bool absolute, const char* font, const char* texture, float height, float scale
            //currentLanguage->name(name).c_str()
            auto btnSpr = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
            auto btnlabel = CCLabelBMFont::create(currentLanguage->name(name).c_str(), "PrismMenu.fnt"_spr);

            Themes::RGBAToCC(PrismUI::GetTheme()["Text"], btnlabel);
            btnlabel->limitLabelWidth(250, 1.0F, .1F);
            btnSpr->setContentSize({ 300.0f, 50.0f });
            btnlabel->setPosition({ btnSpr->getContentSize().width / 2, btnSpr->getContentSize().height / 2 });
            btnSpr->setScale(.5F);
            Themes::RGBAToCC(PrismUI::GetTheme()["Button"], btnSpr);
            btnSpr->addChild(btnlabel);
            auto btn = CCMenuItemSpriteExtra::create(
                btnSpr,
                this,
                menu_selector(PrismUIButton::onBtn)
            );
            btn->setPositionX(63);
            menu->addChild(btn);
            label->removeFromParentAndCleanup(true);
        } else if (type == "dropdown") {
            label->limitLabelWidth(64, 0.5F, .1F);
            auto values = obj.get<matjson::Array>("values");
            if (hack->name == "Theme") {
                values = Themes::getCurrentThemes();
            }
            auto dropdown = Dropdown::create(values, hack, menu_selector(PrismUIButton::onDropdownBtn));
            dropdown->menu->setPosition({78, 0});
            dropdown->menu->setScale(.75F);

            //cocos::handleTouchPriority(dropdown->menu);
            label->setPositionX(180);
            /*for (int i = 0; i < dropdown->menu->getChildrenCount(); i++) {
                auto node = dropdown->menu->getChildren()->objectAtIndex(i);
                if (typeinfo_cast<CCNode*>(node)) {
                    menu->addChild(static_cast<CCNode*>(node));
                }
            }*/
            menu->addChild(dropdown->menu);
        }
    }
    if (m_input != nullptr) {
        Themes::RGBAToCC(PrismUI::GetTheme()["Button"], m_input->getBGSprite());
        m_input->setScale(.65f);
        m_input->setDelegate(this);
        Themes::RGBAToCC(PrismUI::GetTheme()["Text"], m_input->getInputNode()->m_placeholderLabel);
        menu->addChild(m_input);
    }
    menu->setPosition({0,0});
    this->addChild(menu);
    menu->registerWithTouchDispatcher();
    menu->setTouchEnabled(true);
    return true;
}

void PrismUIButton::onBoolBtn(CCObject* ret) {
    auto settings = Mod::get()->getSavedValue<SettingHackStruct>("values");
    std::string name = m_hack->name;
    m_hack->value.boolValue = !m_hack->value.boolValue;
    if (name == "Instant Complete") { // making sure people dont leave this on by accident 
        Hacks::Settings::setSettingValue(&settings, *m_hack, false);
    } else {
        Hacks::Settings::setSettingValue(&settings, *m_hack, m_hack->value.boolValue);
    }
    if (name == "Show Button") {
        auto prismButton = CCScene::get()->getChildByID("prism-icon");
        if (prismButton != nullptr) {
            prismButton->setVisible(m_hack->value.boolValue);
        }
    }
    if (name == "Instant Complete" && m_hack->value.boolValue) {
        FLAlertLayer::create(nullptr, "Cheater!", "Just a warning, you will be <cr>banned off leaderboards</c> if you use this on rated levels. Consider this your <cy>warning</c>.", "OK", nullptr)->show();
    }
}

void PrismUIButton::onFloatBtn(CCObject* ret) {
    auto slider = static_cast<SliderThumb*>(ret);
    auto settings = Mod::get()->getSavedValue<SettingHackStruct>("values");
    std::string name = m_hack->name;
    auto min = m_hack->data.get<int>("min");
    auto max = m_hack->data.get<int>("max");
    float actualValue = getSliderValue(slider->getValue(), min, max, true);
    if (!name.starts_with("Button Position")) {
        m_hack->value.floatValue = actualValue;
    } else {
        m_hack->value.intValue = actualValue;
    }
    m_input->setString(
        name.starts_with("Button Position") ? std::to_string(m_hack->value.intValue) : Utils::setPrecision(m_hack->value.floatValue, 3)
    );
    if (name == "Speedhack") {
        if (m_hack->value.floatValue < 0.0F) return;
        Hacks::setPitch(m_hack->value.floatValue);
        m_hack->value.floatValue = std::max(m_hack->value.floatValue, 0.01f);
    } else {
        if (!name.starts_with("Button Position")) {
            Hacks::Settings::setSettingValue(&settings, *m_hack, m_hack->value.floatValue);
        } else {
            intChanged();
            Hacks::Settings::setSettingValue(&settings, *m_hack, m_hack->value.intValue);
        }
    }
}

void PrismUIButton::onBtn(CCObject* ret) {
    auto settings = Mod::get()->getSavedValue<SettingHackStruct>("values");
    std::string name = m_hack->name;
    auto prismUI = static_cast<PrismUI*>(CCScene::get()->getChildByID("prism-menu"));
    if (name == "Restore Defaults") {
        Hacks::processJSON(true);
        prismUI->onClose(ret);
        //GatoSim::onButton();
    } else if (name == "Import Theme") {
        geode::FileSetting::Filter filter;
        filter.description = "Theme (*.json)";
        filter.files.insert("*.json");
        file::pickFile(
            file::PickMode::OpenFile,
            {
                dirs::getGameDir(),
                { filter }
            },
            [&](ghc::filesystem::path path) {
                auto saveDir = Mod::get()->getSaveDir().string();
                if (!ghc::filesystem::exists(saveDir + "/themes")) {
                    ghc::filesystem::create_directory(saveDir + "/themes");
                }
                auto savePath = saveDir + "/themes/" + path.filename().string();
                if (ghc::filesystem::exists(savePath)) {
                    ghc::filesystem::remove(savePath);
                }
                ghc::filesystem::copy_file(path, savePath); // why this crashes if a file already exists? idk
                FLAlertLayer::create("Success!", "The <cy>theme</c> has successfully been imported! Restart your game to use it.", "OK")->show();
            }
        );
    } else if (name == "Reset Speedhack") {
        HackItem* speedHack = Hacks::getHack("Speedhack");
        speedHack->value.floatValue = 1.0F;
        Hacks::Settings::setSettingValue(&settings, *speedHack, 1.0F);
        Hacks::setPitch(1.0F);
        CCDirector::sharedDirector()->getScheduler()->setTimeScale(1.0F);
        prismUI->onClose(ret);
    } else if (name == "Credits") {
        FLAlertLayer::create("Not working...yet.", "This will be added in a <cy>future update</c>!", "OK")->show();
    } else if (name == "Show Graphic Options") {
        #ifdef GEODE_IS_ANDROID 
        VideoOptionsLayer::create()->show();
        #else 
        FLAlertLayer::create("Error", "This option can only be used on <cy>Android</c>!", "OK")->show();
        #endif
    } else if (name == "Uncomplete Level") {
        if (auto levelInfoLayer = typeinfo_cast<LevelInfoLayer*>(CCScene::get()->getChildByID("LevelInfoLayer"))) {
            // i forgor why i didnt do this and did for loop
            Hacks::resetLevel(levelInfoLayer, levelInfoLayer->m_level);
        } else {
            FLAlertLayer::create("Error", "You are not <cy>currently in the level page</c>! Please enter in a level page in order to <cg>reset the stats</c>.", "OK")->show();
        }
    } else {
        // NO SPOILERS!
        GatoSim::onButton();
    }
}

void PrismUIButton::onDropdownBtn(CCObject* sender) {
    auto obj = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto hack = static_cast<HackItem*>(obj->getUserData());
    auto idStr = obj->getID();
    static_cast<CCLabelBMFont*>(obj->getParent()->getParent()->getChildByID("selected-label"))->setString((static_cast<CCLabelBMFont*>(obj->getChildren()->objectAtIndex(0)))->getString());

    auto obj2 = static_cast<CCMenuItemSprite*>(obj->getParent()->getParent()->getChildByID("flip-btn"));
    bool expanded = obj2->getScaleY() < 0 ? true : false;
    #ifdef GEODE_IS_WINDOWS
    obj2->runAction(CCEaseBackOut::create(CCScaleTo::create(0.5f, 0.75f, (!expanded ? -0.75f : 0.75f))));
    #else
    #ifdef GEODE_IS_ANDROID
    obj2->runAction(CCScaleTo::create(0.5f, 0.75f, (!expanded ? -0.75f : 0.75f)));
    #endif
    #endif

    auto parent = obj2->getParent();
    auto background = parent->getChildByID("background");
    auto ddmenu = parent->getChildByID("dropdown-menu");

    int h = ddmenu->getChildrenCount() + 1;
    h = expanded ? h : 1;

    ddmenu->setVisible(expanded);
    background->setContentSize({background->getContentSize().width, (25 / background->getScale()) * h});

    // chat jippity
    // Splitting the string by "-"
    std::istringstream ss(idStr);
    std::string token;
    std::vector<std::string> tokens;
    
    while (std::getline(ss, token, '-')) {
        tokens.push_back(token);
    }
    if (!tokens.empty()) {
        std::string lastElement = tokens.back();
        int index = std::stoi(lastElement);
        auto settings = Mod::get()->getSavedValue<SettingHackStruct>("values");
        log::debug("Set Dropdown Value to {}", index);
        hack->value.intValue = index;
        Hacks::Settings::setSettingValue(&settings, *hack, hack->value.intValue);
        if (hack->name == "Menu-Style") {
            auto obj = static_cast<PrismUI*>(CCScene::get()->getChildByID("prism-menu"));
            obj->onClose(sender);
        } else if (hack->name == "Theme") {
            currentThemeApplied = matjson::Array {};
        }
    }
}

void PrismUI::CreateHackItem(HackItem* hack) {
    const auto& obj = hack->data;
    std::string name = hack->name;
    std::string desc = hack->desc;
    auto opcodes = obj.get<matjson::Array>("opcodes");
    if (!((Hacks::isHackEnabled("Enable Patching") && obj.contains("winOnly")) || !obj.contains("winOnly") || name == "Enable Patching")) return;
    auto btn = PrismUIButton::create(hack, m_currentLang.get());
    float indexY = (currentI * -28) + 310;
    // TODO: create custom sprite so people dont complain
    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    //infoSpr->setScale(.5F);
    auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(PrismUIButton::onInfoBtn));
    infoBtn->setUserData(reinterpret_cast<void*>(hack));
    m_content->addChild(infoBtn);
    
    btn->setPositionY(indexY);
    m_content->addChild(btn);
    // thingy i was struggling with
    if (m_content->getContentSize().height > 230.0F) {
        //m_scrollLayer->m_contentLayer->setContentSize(m_content->getContentSize() + CCSize { 0.f, 1.0 });
        m_scrollLayer->m_contentLayer->setContentSize(m_content->getContentSize());
    } else {
        m_scrollLayer->m_contentLayer->setContentSize(m_content->getContentSize());
    }
    m_scrollLayer->m_contentLayer->setContentSize({m_content->getContentSize().width, getContentSizeBasedOnCategory(currentI + 1)});
    infoBtn->setPosition({280, indexY});
    //m_content->setPositionY(calculateYPosition(currentI));
    m_content->setPositionY(getYPosBasedOnCategory(currentI + 1));
    m_scrollLayer->moveToTop();
    currentI++;
}

void PrismUIButton::textChanged(CCTextInputNode* input) {
    if (!editedInputNode) return;
    Themes::RGBAToCC(PrismUI::GetTheme()["Text"], input->m_placeholderLabel);
    const auto& obj = m_hack->data;
    auto settings = Mod::get()->getSavedValue<SettingHackStruct>("values");
    std::string name = m_hack->name;
    auto min = m_hack->data.get<int>("min");
    auto max = m_hack->data.get<int>("max");
    if (m_hack->value.type == ValueType::Int) {
        int value = min;
        std::istringstream iss(input->getString());
        if (!(iss >> value)) {
            auto def = m_hack->data.get<int>("default");
            value = def;
        }
        if (value == m_hack->value.intValue) return;
        if (m_hack->value.intValue > max || min > m_hack->value.intValue) {
            if (m_hack->value.intValue > max) {
                m_hack->value.intValue = max;
            } else {
                m_hack->value.intValue = min;
            }
        } else {
            m_hack->value.intValue = value;
        }
        Hacks::Settings::setSettingValue(&settings, *m_hack, m_hack->value.intValue);
    } else { // assume float
        float value = min;
        std::istringstream iss(input->getString());
        if (!(iss >> value)) {
            auto def = m_hack->data.get<float>("default");
            value = def;
        }
        if (value == m_hack->value.floatValue) return;
        if (m_hack->value.floatValue > max || min > m_hack->value.floatValue) {
            if (m_hack->value.floatValue > max) {
                m_hack->value.floatValue = max;
            } else {
                m_hack->value.floatValue = min;
            }
        } else {
            m_hack->value.floatValue = value;
        }
        Hacks::Settings::setSettingValue(&settings, *m_hack, m_hack->value.floatValue);
    }
}
void PrismUIButton::intChanged() {
    std::string name = m_hack->name;
    auto prismButton = CCScene::get()->getChildByID("prism-icon");
    if (name == "FPS Bypass") {
        // from mats fps unlocker
        //Hacks::Settings::setSettingValue(&settings, *hack, hack->value.floatValue);
#ifndef GEODE_IS_MACOS // crashes
        auto app = CCApplication::sharedApplication();
        app->setAnimationInterval(1.0 / static_cast<double>(m_hack->value.intValue));
#endif 
        auto GM = GameManager::sharedState();
        GM->m_customFPSTarget = m_hack->value.intValue;
        GM->setGameVariable("0116", true);
        GM->updateCustomFPS();
    } else if (name == "Button Position X") {
        prismButton->setPositionX(m_hack->value.intValue);
    } else if (name == "Button Position Y") {
        prismButton->setPositionY(m_hack->value.intValue);
    }
}

void PrismUIButton::textInputOpened(CCTextInputNode* input) { // basically onIntBtn
    if (input->getString().size() == 0) return;
    editedInputNode = true;
    Themes::RGBAToCC(PrismUI::GetTheme()["Text"], input->m_placeholderLabel);
}

void PrismUIButton::textInputClosed(CCTextInputNode* input) { // basically onIntBtn
    if (input->getString().size() == 0) return;
    editedInputNode = false;
    std::string name = m_hack->name;
    Themes::RGBAToCC(PrismUI::GetTheme()["Text"], input->m_placeholderLabel);
    if (m_hack->type == "float") {
        input->setString(Utils::setPrecision(m_hack->value.floatValue, 3));
        if (name == "Speedhack") {
            if (m_hack->value.floatValue < 0.0F) return;
            m_hack->value.floatValue = std::max(m_hack->value.floatValue, 0.01f);
            Hacks::setPitch(m_hack->value.floatValue);
#ifdef GEODE_IS_WINDOWS
            CCDirector::sharedDirector()->getScheduler()->setTimeScale(m_hack->value.floatValue);
#endif
        }
    } else {
        input->setString(std::to_string(m_hack->value.intValue));
        intChanged();
    }
}

void PrismUIButton::onIncBtn(CCObject* ret) {
    auto settings = Mod::get()->getSavedValue<SettingHackStruct>("values");
    const auto& obj = m_hack->data;
    auto max = m_hack->data.get<int>("max");
    int step = (obj.contains("step")) ? obj.get<int>("step") : 5;
    m_hack->value.intValue += step;
    if (m_hack->value.intValue > max) {
        m_hack->value.intValue = max;
    }
    Hacks::Settings::setSettingValue(&settings, *m_hack, m_hack->value.intValue);
    m_input->setString(std::to_string(m_hack->value.intValue));
    intChanged();
}
void PrismUIButton::onDecBtn(CCObject* ret) {
    auto settings = Mod::get()->getSavedValue<SettingHackStruct>("values");
    const auto& obj = m_hack->data;
    auto min = m_hack->data.get<int>("min");
    int step = (obj.contains("step")) ? obj.get<int>("step") : 5;
    m_hack->value.intValue -= step; 
    if (min > m_hack->value.intValue) {
        m_hack->value.intValue = min;
    }
    Hacks::Settings::setSettingValue(&settings, *m_hack, m_hack->value.intValue);
    m_input->setString(std::to_string(m_hack->value.intValue));
    intChanged();
}

void PrismUI::RegenCategory() {
    std::vector<matjson::Value> jsonArray;
    currentI = 0;
    switch (currentMenuIndexGD) {
        case 0: // Global 
            jsonArray = matjson::parse(Hacks::getGlobalHacks()).as_array();
            break;
        case 1: // Player
            jsonArray = matjson::parse(Hacks::getPlayerHacks()).as_array();
            break;
        case 2: // Bypass
            jsonArray = matjson::parse(Hacks::getBypassHacks()).as_array();
            break;
        case 3: // Creator
            jsonArray = matjson::parse(Hacks::getCreatorHacks()).as_array();
            break;
        case 4: // Misc
            jsonArray = matjson::parse(Hacks::getMiscHacks()).as_array();
            break;
        case 5: { // Settings
            jsonArray = matjson::parse(Hacks::getSettings()).as_array();
            auto createdByLabel = CCLabelBMFont::create(m_currentLang->name("Prism Menu by Firee").c_str(), "PrismMenu.fnt"_spr);
            auto versionLabel = CCLabelBMFont::create("Unknown.", "PrismMenu.fnt"_spr);
            float indexY = (currentI * -28) + 310;
            createdByLabel->limitLabelWidth(150, 1.0F, .2F);
            createdByLabel->setPosition({63, indexY});
            versionLabel->setPosition({63, indexY + 15});
            Themes::RGBAToCC(GetTheme()["Text"], createdByLabel);
            Themes::RGBAToCC(GetTheme()["Text"], versionLabel);
            #ifndef DEV_BUILD
            auto version = fmt::format("{} (Geode)", Mod::get()->getVersion().toString());
            #else 
            auto version = fmt::format("{}-Dev (Geode)", Mod::get()->getVersion().toString());
            #endif

            #ifdef GEODE_IS_WINDOWS
                versionLabel->setString(fmt::format("{} - Windows", version).c_str());
            #else // why does android not like elif
            #ifdef GEODE_IS_ANDROID
                versionLabel->setString(fmt::format("{} - Android", version).c_str());
            #else 
            #ifdef GEODE_IS_MACOS
                versionLabel->setString(fmt::format("{} - Mac OS", version).c_str());
            #else 
                versionLabel->setString(fmt::format("{} - HOW by Spu7nix", version).c_str());
            #endif
            #endif
            #endif
            versionLabel->limitLabelWidth(150, 1.0F, .2F);
            m_content->addChild(createdByLabel);
            m_content->addChild(versionLabel);
            currentI++;
            break;
        }
    }
    for (auto it = jsonArray.begin(); it != jsonArray.end(); it++) {
        const auto& obj = *it;
        std::string name = obj.get<std::string>("name");
        HackItem* hack = Hacks::getHack(name);
        if (hack != nullptr) {
            CreateHackItem(hack);
        }
    }
}
void PrismUI::CreateButton(const char* name, int menuIndex) {
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
    auto menu = CCMenu::create();
    auto node = CCLayer::create();

    auto label = CCLabelBMFont::create(name, "PrismMenu.fnt"_spr);
    //label->addChild(invisBG);
    label->limitLabelWidth(72, 1.0F, .2F);
    auto button = CCMenuItemSpriteExtra::create(label, this, menu_selector(PrismUI::onSideButton));
    button->setContentSize({90.0f, 35.0f});
    label->setPosition({90.0f / 2, 35.0f / 2});
    auto buttonBG = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    buttonBG->setContentSize({ 90.0f, 35.0f });
    Themes::RGBAToCC(GetTheme()["Button"], buttonBG);
    Themes::RGBAToCC(GetTheme()["Text"], button);
    if (currentMenuIndexGD == menuIndex) {
        button->setColor({255,255,255});
        Themes::RGBAToCC(GetTheme()["ButtonActive"], buttonBG);
    }
    //menu->addChild(buttonBG);
    buttonBG->setPosition(-163, (menuIndex * -38) + 97);
    button->setPosition(-163, (menuIndex * -38) + 97);
    buttonBG->setID(fmt::format("prism-nav-bg-{}", menuIndex));
    button->setID(fmt::format("prism-nav-btn-{}", menuIndex));
    button->setUserData(button);
    m_buttonMenu->addChild(buttonBG);
    m_buttonMenu->addChild(button);
    //menu->registerWithTouchDispatcher();
}



void PrismUI::onSideButton(CCObject* ret) {
    m_content->removeAllChildrenWithCleanup(true);
    auto idStr = static_cast<CCNodeRGBA*>(ret)->getID();
    int id = std::stoi(idStr.substr(idStr.length() - 1));
    ButtonState(currentMenuIndexGD, false);
    ButtonState(id, true);
    RegenCategory();
    cocos::handleTouchPriority(this);
}


double calculateTextScale(const std::string& inputString, double minScale = 0.2, double maxScale = 0.5) {
    int numCharacters = static_cast<int>(inputString.size());
    double scale = minScale + (numCharacters / 100.0) * (maxScale - minScale); // Assuming a range of 0-100 characters
    return std::max(minScale, std::min(scale, maxScale));
}

void PrismUIButton::onInfoBtn(CCObject* ret) {
    HackItem* hack = static_cast<HackItem*>(static_cast<PrismUIButton*>(ret)->getUserData());
    auto currentLanguage = Lang::getLanguage();
    auto name = currentLanguage->name(hack->name);
    auto desc = currentLanguage->desc(hack->name, hack->desc);
    auto flAlert = FLAlertLayer::create(name.c_str(), "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "OK"); // internal screaming
    TextArea* lines;
    CCLabelBMFont* title;
    for (int i = 0; i < flAlert->m_mainLayer->getChildrenCount(); i++) {
        auto objC = flAlert->m_mainLayer->getChildren()->objectAtIndex(i); // objective-c yes
        if (Utils::getNodeName(objC) == "TextArea") {
            lines = static_cast<TextArea*>(objC);
        } else if (Utils::getNodeName(objC) == "cocos2d::CCLabelBMFont") {
            title = static_cast<CCLabelBMFont*>(objC);
        }
    }
    if (lines != nullptr && title != nullptr) {
        // probably a bad idea memory-wise
        auto newTitle = CCLabelBMFont::create(name.c_str(), "PrismMenu.fnt"_spr);
        newTitle->setPosition({title->getPositionX(), title->getPositionY() - 15});
        newTitle->limitLabelWidth(256, 0.75F, 0.25F);
        newTitle->setZOrder(title->getZOrder());
        Themes::RGBAToCC(PrismUI::GetTheme()["ButtonActive"], newTitle);
        title->removeFromParentAndCleanup(true);
        flAlert->m_mainLayer->addChild(newTitle);
        /*auto newLines = TextArea::create(
            hack->desc.c_str(), "PrismMenu.fnt"_spr, 1.0F,
            240.F, { 0.5, 0 }, 20.F, false);*/
        // broken with cyrillic and other accents :(
        /*auto newLines = TextArea::create(
            desc.c_str(), "PrismMenu.fnt"_spr, 0.7F, //0.8
            280.F, { 0.5, 0 }, 20.F, false);*/ 
        //const std::string& font, const std::string& text, const float scale, const float width,
        auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
        auto newLines = SimpleTextArea::create("A", "PrismMenu.fnt"_spr, 0.5F, winSize.width / 2.2F); // 260.0F
        newLines->setText(desc.c_str());
        newLines->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
        newLines->setScale(calculateScale(desc, 25, .75F, .25F));
        // ok geode broke AxisLayout, thats nice
        newLines->setPosition({(winSize.width / 2), (winSize.height / 2) + 10}); // 160 - 220
        newLines->setZOrder(lines->getZOrder());
        lines->removeFromParentAndCleanup(true);
        flAlert->m_mainLayer->addChild(newLines);
        flAlert->show();
    }

}

matjson::Object PrismUI::GetTheme() {
    if (currentThemeApplied.empty()) {
        log::debug("Getting current theme...");
        currentThemeApplied.push_back(Themes::getCurrentTheme());
    }
    return currentThemeApplied[0].as_object();
}

bool PrismUI::init(float _w, float _h) {
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
    m_currentLang = Lang::getLanguage();

    float menuScale = Hacks::getHack("Menu Scale")->value.floatValue;
    this->m_pLrSize = cocos2d::CCSize { _w, _h };
    //  this->m_pLrSize = cocos2d::CCSize { _w * menuScale, _h * menuScale };
    if (!this->initWithColor({ 0, 0, 0, 105 })) return false;
    this->m_mainLayer = cocos2d::CCLayer::create();
    this->addChild(m_mainLayer);

    auto bg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    auto bgBehind = cocos2d::extension::CCScale9Sprite::create("GJ_square07.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    bg->setContentSize(m_pLrSize - 3);
    bgBehind->setContentSize(m_pLrSize);
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    bgBehind->setPosition(winSize.width / 2, winSize.height / 2);

    m_mainLayer->addChild(bgBehind);
    m_mainLayer->addChild(bg);
    bg->setID("prism-bg");
    this->m_pBGSprite = bg;

    this->m_buttonMenu = cocos2d::CCMenu::create();
    m_mainLayer->addChild(this->m_buttonMenu);

    // TODO: create custom sprite so people dont complain
    auto closeSpr = cocos2d::CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    closeSpr->setScale(1.0f);

    auto closeBtn = CCMenuItemSpriteExtra::create(
        closeSpr,
        this,
        (cocos2d::SEL_MenuHandler)&PrismUI::onClose
    );
    closeBtn->setUserData(reinterpret_cast<void*>(this));

    this->m_buttonMenu->addChild(closeBtn);

    closeBtn->setPosition( - _w / 2, _h / 2 );
    
    auto prismButton = CCScene::get()->getChildByID("prism-icon");
    if (prismButton != nullptr) {
        static_cast<CCMenuItemSpriteExtra*>(prismButton->getChildren()->objectAtIndex(0))->setEnabled(false);
    }

    auto sideBar = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    sideBar->setContentSize({ 100.0f, 230.0f });
    //sideBar->setPosition(125, bg->getContentSize().height / 2);
    sideBar->setPosition(60, _h / 2);
    Themes::RGBAToCC(GetTheme()["TableRowBg"], sideBar);

    auto buttonBG = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    buttonBG->setContentSize({ 320.0f, 230.0f });
    buttonBG->setPosition(_w - 173, _h / 2);
    Themes::RGBAToCC(GetTheme()["TableRowBg"], buttonBG);

    bg->addChild(sideBar);
    bg->addChild(buttonBG);
    CreateButton(m_currentLang->name("§ Global").c_str(), 0);
    CreateButton(m_currentLang->name("¬ Player").c_str(), 1);
    CreateButton(m_currentLang->name("ª Bypass").c_str(), 2);
    CreateButton(m_currentLang->name("« Creator").c_str(), 3);
    CreateButton(m_currentLang->name("··· Misc").c_str(), 4);
    CreateButton(m_currentLang->name("¶ Settings").c_str(), 5);
    Themes::RGBAToCC(GetTheme()["BG"], bg);
    Themes::RGBAToCC(GetTheme()["BG"], bgBehind);
    bgBehind->setColor({255, 255, 255});
        
    m_scrollLayer = ScrollLayer::create({ 0, 0, 320.0F, 230.0F }, true);
    //m_scrollLayer->setPosition(_w - 273, 45);
    m_content = CCMenu::create();
    m_content->setZOrder(2);
    m_content->setPositionX(20);
    m_content->registerWithTouchDispatcher();

    m_scrollLayer->m_contentLayer->addChild(m_content);

    m_scrollLayer->setTouchEnabled(true);

    buttonBG->addChild(m_scrollLayer);
    this->registerWithTouchDispatcher();

    cocos::handleTouchPriority(this);
    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);
    this->setID("prism-menu");
    RegenCategory();
    return true;
}

void PrismUI::keyDown(cocos2d::enumKeyCodes key) {
    if (key == cocos2d::enumKeyCodes::KEY_Escape)
        return onClose(nullptr);
    if (key == cocos2d::enumKeyCodes::KEY_Tab)
        return onClose(nullptr);
    if (key == cocos2d::enumKeyCodes::KEY_Space)
        return;
    
    return FLAlertLayer::keyDown(key);
}

// why is onClose not actually called what!?
void PrismUI::keybackClicked() {
    onClose(nullptr);
};

void PrismUI::onClose(cocos2d::CCObject* pSender) {
    auto prismButton = CCScene::get()->getChildByID("prism-icon");
    if (prismButton != nullptr) {
        static_cast<CCMenuItemSpriteExtra*>(prismButton->getChildren()->objectAtIndex(0))->setEnabled(true);
    }
    if (PlayLayer::get() != nullptr) { // attempt to fix the stupid issue
        #ifndef GEODE_IS_DESKTOP
        for (size_t i = 0; i < CCScene::get()->getChildrenCount(); i++) {
            auto obj = CCScene::get()->getChildren()->objectAtIndex(i);
            if (i > 10) break;
            if (Utils::getNodeName(obj) == "PauseLayer") {
                log::debug("Gave touch priority back to PauseLayer");
                auto pauseLayer = static_cast<PauseLayer*>(obj);
                cocos::handleTouchPriority(pauseLayer);
                break;
            }
        }
        #endif
    }
    this->removeFromParentAndCleanup(true);
};

PrismUI* PrismUI::create() {
    auto pRet = new PrismUI();
    if (pRet) {
        if (pRet->init(PrismUI::s_defWidth, PrismUI::s_defHeight)) {
            pRet->autorelease();
            return pRet;
        }
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
};

PrismUIButton* PrismUIButton::create(HackItem* hack, Lang* lang) {
    auto pRet = new PrismUIButton();
    if (pRet) {
        pRet->currentLanguage = lang;
        if (pRet->init(hack)) {
            pRet->autorelease();
            return pRet;
        }
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrismDynamicUIButton::test(CCObject* obj) {
    typeinfo_cast<CCNode*>(obj)->setVisible(false);
}

bool PrismDynamicUIButton::init(HackItem* hack) {

    if (!CCMenu::init()) return false;

    CCSize size(70.0f, 8.0f);
    CCRect sizeRect(0, 0, size.width, size.height);
    float iconPad = 1.2f;

    std::string hackName = hack ? hack->name : "LABEL";

    CCSprite* icon = nullptr;
    if (hack && (hack->type == "float" || hack->type == "int")) {
        icon = CCSprite::create("hashtag.png"_spr);
        icon->setScale((size.height - iconPad * 2) / icon->getContentHeight());
        icon->setAnchorPoint(CCPoint(0.0f, 0.0f));
        icon->setPosition(CCPoint(iconPad, iconPad));
        icon->setColor({ 150, 150, 150 });
    }

    float labelPadLeft = 1.0f + size.height;
    m_label = SimpleTextArea::create(hackName, "PrismMenu.fnt", 0.7f, size.width - labelPadLeft);
//    while (!hackName.empty() && m_label->getLines().size() > 1) {
//        hackName.pop_back();
//        if (hackName.ends_with(' ')) hackName.pop_back();
//        m_label->setText(hackName + "...");
//    }

    auto anchor = CCPoint(0.0f, 0.5f);

    if (!m_label) return false;
    m_label->setAnchorPoint(anchor);
    m_label->setContentHeight(m_label->getLineHeight());
    m_label->setPosition(CCPoint(labelPadLeft, size.height / 2));

    auto bgNormal = CCSprite::create("square.png", sizeRect);
    auto bgSelected = CCSprite::create("square.png", sizeRect);
    bgNormal->setColor({ .r = 50, .g = 50, .b = 50 });
    bgSelected->setColor({ .r = 40, .g = 40, .b = 40 });
    m_background = CCMenuItemSprite::create(
            bgNormal, bgSelected, this, menu_selector(PrismDynamicUIButton::test)
    );

    if (!m_background) return false;

    m_background->setAnchorPoint(anchor);
    m_background->setPositionY(size.height / 2);

    this->addChild(m_background);
    if (icon) this->addChild(icon);
    this->addChild(m_label);

    this->setContentSize(size);
    this->ignoreAnchorPointForPosition(false);
    this->setAnchorPoint(CCPoint(0.0f, 1.0f));

    this->registerWithTouchDispatcher();
    this->setTouchEnabled(true);

    return true;

}

bool PrismDynamicUILabel::init(PrismDynamicUIMenu* menu, std::string text) {
    if (!PrismDynamicUIButton::init(nullptr)) return false;
    m_menu = menu;
    m_label->setText(text);
    m_label->setAlignment(kCCTextAlignmentCenter);
    typeinfo_cast<CCSprite*>(m_background->getNormalImage())->setColor({ 60, 60, 255 });
    typeinfo_cast<CCSprite*>(m_background->getSelectedImage())->setColor({ 58, 58, 245 });
    return true;
}

bool PrismDynamicUILabel::ccTouchBegan(CCTouch* touch, CCEvent*) {
    m_dragging = this->boundingBox().containsPoint(touch->getLocation());
    return true;
}

void PrismDynamicUILabel::ccTouchEnded(CCTouch* touch, CCEvent*) {
    if (m_dragging && this->boundingBox().containsPoint(touch->getLocation()))
        m_dragging = false;
}

void PrismDynamicUILabel::ccTouchCancelled(CCTouch* p0, CCEvent* p1) {
    this->ccTouchEnded(p0, p1);
}

void PrismDynamicUILabel::ccTouchMoved(CCTouch* touch, CCEvent*) {
    std::stringstream s;
    s << m_menu->getPositionX() << ", " << m_menu->getPosition(Y) << " vs ";
    s << touch->getPositionX() << ", " << touch->getPosition(Y);
    if (m_dragging) m_menu->setPosition(m_menu->getPosition() + touch->getDelta());
}



PrismDynamicUILabel* PrismDynamicUILabel::create(PrismDynamicUIMenu* menu, std::string text) {
    auto ret = new PrismDynamicUILabel();
    if (ret && ret->init(menu, text)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

PrismDynamicUIButton* PrismDynamicUIButton::create(HackItem* hack) {
    auto ret = new PrismDynamicUIButton();
    if (ret && ret->init(hack)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool PrismDynamicUIMenu::init() {

    if (!CCMenu::init()) return false;

    auto head = PrismDynamicUILabel::create(this, "Label");
    if (!head) return false;
    this->addChild(head);

    this->updateButtons();

    return true;
}

void PrismDynamicUIMenu::updateButtons() {

    auto children = this->getChildren();
    auto childCount = children->count();

    if (childCount == 0) {
        this->setContentSize(CCSize(0, 0));
        return;
    }

    auto firstButton = typeinfo_cast<CCNode*>(children->objectAtIndex(0)); // no firstObject; geode is failing me
    float buttonHeight = firstButton->getContentHeight(); // assume all buttons have the same height
    float menuHeight = (float)childCount * buttonHeight;
    this->setContentSize(CCSize(firstButton->getContentWidth(), menuHeight));

    for (int i = 0; i < childCount; ++i) {
        auto button = typeinfo_cast<CCNode*>(children->objectAtIndex(i));
        button->setPosition(CCPoint(0.0f, menuHeight - buttonHeight * (float)i));
    }

    this->setAnchorPoint(CCPoint(0.0f, 1.0f));
    this->ignoreAnchorPointForPosition(false);

}

PrismDynamicUIMenu* PrismDynamicUIMenu::create() {
    auto ret = new PrismDynamicUIMenu();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool PrismDynamicUI::init() {
    if (!CCLayer::init()) return false;
    this->setID("prism-menu");

    auto menu = PrismDynamicUIMenu::create();
    if (!menu) return false;
    menu->setPosition(CCScene::get()->getContentSize() / 2);

    for (auto& hack : allHacks) {
        auto button = PrismDynamicUIButton::create(&hack);
        if (!button) return false;
        menu->addChild(button);
    }
    menu->updateButtons();

    this->addChild(menu);

    return true;
}

PrismDynamicUI* PrismDynamicUI::create() {
    auto ret = new PrismDynamicUI();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}