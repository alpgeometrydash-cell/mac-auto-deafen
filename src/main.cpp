#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "keypress.hpp"

using namespace geode::prelude;

// Discord's keybind is a *toggle*, so we track whether we've deafened you.
static bool g_deafened = false;

static void pressDeafenKey() {
    auto mod = Mod::get();
    auto key = mod->getSettingValue<std::string>("key");
    pressKeyCombo(
        key.empty() ? 'D' : key[0],
        mod->getSettingValue<bool>("ctrl"),
        mod->getSettingValue<bool>("option"),
        mod->getSettingValue<bool>("shift"),
        mod->getSettingValue<bool>("command")
    );
}

static void deafen() {
    if (g_deafened) return;
    g_deafened = true;
    log::info("Deafening");
    pressDeafenKey();
}

static void undeafen() {
    if (!g_deafened) return;
    g_deafened = false;
    log::info("Undeafening");
    pressDeafenKey();
}

$on_mod(Loaded) {
    // Pops up the macOS "allow Accessibility" prompt if GD isn't allowed to send keys yet.
    if (!hasAccessibilityPermission(true)) {
        log::warn("No Accessibility permission - keypresses to Discord will be blocked");
    }
}

class $modify(AutoDeafenPlayLayer, PlayLayer) {
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (g_deafened) return;
        auto mod = Mod::get();
        if (!mod->getSettingValue<bool>("enabled")) return;
        if (m_isPlatformer) return; // no % in platformer
        if (m_isPracticeMode && !mod->getSettingValue<bool>("practice-mode")) return;
        if (m_startPosObject && mod->getSettingValue<bool>("ignore-startpos")) return;
        if (!m_player1 || m_player1->m_isDead || m_levelEndAnimationStarted) return;

        auto target = mod->getSettingValue<int64_t>("percent");
        if (this->getCurrentPercent() >= static_cast<float>(target)) {
            deafen();
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        // GD calls this once at level start with a fake "anticheat" object,
        // so only undeafen if you actually died.
        if (m_player1 && m_player1->m_isDead) {
            undeafen();
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        undeafen();
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        if (Mod::get()->getSettingValue<bool>("undeafen-on-complete")) {
            undeafen();
        }
    }

    void onQuit() {
        undeafen();
        PlayLayer::onQuit();
    }
};
