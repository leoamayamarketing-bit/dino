#include "../../include/entities/PlayerFactory.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/PhysicsComponent.h"
#include "../../include/components/CollisionComponent.h"
#include "../../include/components/PlayerComponent.h"
#include "../../include/components/AnimationComponent.h"

std::unique_ptr<Entity> PlayerFactory::createPlayer(
    Constants::DinoType type, AssetManager& assets, float startX, float startY) {

    auto entity = std::make_unique<Entity>(1);

    std::string texName;
    switch (type) {
        case Constants::DinoType::TREX:
            texName = "dino_trex"; break;
        case Constants::DinoType::VELOCIRAPTOR:
            texName = "dino_raptor"; break;
        case Constants::DinoType::TRICERATOPS:
            texName = "dino_triceratops"; break;
    }

    entity->addComponent<TransformComponent>(startX, startY);
    auto& sprite = entity->addComponent<SpriteComponent>(texName);
    sprite.zOrder = 10;

    // ─── Sprite origin: bottom-center ────────────────────────────────────
    // This makes the transform position represent the dino's FEET/ground
    // contact point instead of the top-left corner. The dino will stand ON
    // the ground correctly regardless of sprite size.
    //
    // To change the character: just swap the PNG files in assets/.
    // The origin is automatically derived from the texture dimensions.
    auto setOriginBottom = [](SpriteComponent& s) {
        if (s.sprite.getTexture()) {
            sf::Vector2u texSize = s.sprite.getTexture()->getSize();
            s.sprite.setOrigin(texSize.x / 2.0f, static_cast<float>(texSize.y));
        }
    };

    auto& anim = entity->addComponent<AnimationComponent>();

    // Check if we have loaded PNG animation frames
    const auto& dinoFrames = assets.getDinoFrameNames();
    bool hasPNGFrames = !dinoFrames.empty();

    if (hasPNGFrames) {
        // ─── Use loaded PNG sprites for animation ─────────────────────────
        // The PNG frames are named dino_frame_00 through dino_frame_08
        // Walk: cycle through frames 01, 02, 03
        // Jump: use frame 00 (mid-air pose)
        // Crouch: progressive 01→07, stand: frame 08

        // Helper lambda to get texture size
        auto getTexSize = [&assets](const std::string& name) -> sf::Vector2u {
            if (assets.hasTexture(name)) {
                return assets.getTexture(name).getSize();
            }
            return sf::Vector2u(128, 72);
        };

        anim.useSeparateTextures = true;

        // --- Walk animation: cycle frames 01, 02, 03 ---
        if (dinoFrames.size() >= 4) {
            anim.addAnimation("run", {
                {sf::IntRect(0, 0, static_cast<int>(getTexSize(dinoFrames[1]).x),
                             static_cast<int>(getTexSize(dinoFrames[1]).y)), 0.12f, dinoFrames[1]},
                {sf::IntRect(0, 0, static_cast<int>(getTexSize(dinoFrames[2]).x),
                             static_cast<int>(getTexSize(dinoFrames[2]).y)), 0.12f, dinoFrames[2]},
                {sf::IntRect(0, 0, static_cast<int>(getTexSize(dinoFrames[3]).x),
                             static_cast<int>(getTexSize(dinoFrames[3]).y)), 0.12f, dinoFrames[3]},
            });
        } else {
            // Fallback if not enough PNGs
            anim.addAnimation("run", {
                {sf::IntRect(0, 0, 128, 72), 0.12f},
                {sf::IntRect(128, 0, 128, 72), 0.12f}
            });
        }

        // --- Jump animation: use frame 00 ---
        if (dinoFrames.size() >= 1) {
            anim.addAnimation("jump", {
                {sf::IntRect(0, 0, static_cast<int>(getTexSize(dinoFrames[0]).x),
                             static_cast<int>(getTexSize(dinoFrames[0]).y)), 0.15f, dinoFrames[0]}
            });
        } else {
            anim.addAnimation("jump", {
                {sf::IntRect(256, 0, 128, 72), 0.15f}
            });
        }

        // --- Crouch animation: progressive frames 01→07 ---
        // Note: textureName is NOT set on crouch frames because
        // PlayerControlSystem handles texture switching directly
        // for progressive crouch animation (to avoid RenderSystem overriding it).
        if (dinoFrames.size() >= 4) {
            // Use first crouch frame as initial (no textureName - PlayerControlSystem handles it)
            anim.addAnimation("crouch", {
                {sf::IntRect(0, 0, static_cast<int>(getTexSize(dinoFrames[4]).x),
                             static_cast<int>(getTexSize(dinoFrames[4]).y)), 0.10f, ""}
            });
        } else {
            anim.addAnimation("crouch", {
                {sf::IntRect(384, 0, 128, 72), 0.15f}
            });
        }

        // --- Stand up animation: single frame 08 ---
        // Same as crouch: no textureName, PlayerControlSystem handles it.
        if (dinoFrames.size() >= 9) {
            anim.addAnimation("stand", {
                {sf::IntRect(0, 0, static_cast<int>(getTexSize(dinoFrames[8]).x),
                             static_cast<int>(getTexSize(dinoFrames[8]).y)), 0.10f, ""}
            });
        }

        // Set initial texture to first PNG frame
        if (assets.hasTexture(dinoFrames[1])) {
            sprite.setTexture(assets.getTexture(dinoFrames[1]));
        } else if (assets.hasTexture(texName)) {
            sprite.setTexture(assets.getTexture(texName));
        }
        // Set origin AFTER setting texture
        setOriginBottom(sprite);

    } else {
        // ─── Fallback: use procedural sprite sheet ──────────────────────
        if (assets.hasTexture(texName)) {
            sprite.setTexture(assets.getTexture(texName));
        }
        setOriginBottom(sprite);

        anim.addAnimation("run", {
            {sf::IntRect(0, 0, 128, 72), 0.12f},    // run pose 1
            {sf::IntRect(128, 0, 128, 72), 0.12f}    // run pose 2
        });
        anim.addAnimation("jump", {
            {sf::IntRect(256, 0, 128, 72), 0.15f}    // jump pose (single frame)
        });
        anim.addAnimation("crouch", {
            {sf::IntRect(384, 0, 128, 72), 0.15f}    // crouch pose (single frame)
        });
    }

    anim.play("run");
    anim.looping = true;
    anim.speed = 1.5f;

    auto& physics = entity->addComponent<PhysicsComponent>();
    physics.usesGravity = true;
    physics.jumpsAvailable = 2;

    // Collision box adjusted for bottom-center origin.
    // CollisionSystem updates bounds.left/top from transform position + localOffset.
    // With origin at bottom-center (startX, startY = GROUND_Y), the collision
    // box is offset upward to cover the dino body above the feet.
    auto& coll = entity->addComponent<CollisionComponent>(
        sf::FloatRect(0, 0, 70, 55), "player");
    coll.localOffset = sf::Vector2f(-35.0f, -65.0f);
    coll.isTrigger = false;
    coll.isStatic = false;

    entity->addComponent<PlayerComponent>(type);

    return entity;
}
