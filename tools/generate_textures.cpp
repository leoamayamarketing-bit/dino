// DinoRunner Professional Texture Generator
// Generates high-quality sprite sheets and textures at multiple sizes
// Compile: g++ -std=c++17 -DSFML_STATIC -I ../include -I ../sfml/include generate_textures.cpp ../src/abi_compat.cpp -L ../sfml/lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lfreetype -lstdc++ -lws2_32 -lwinmm -lopengl32 -lgdi32 -lole32 -loleaut32 -luuid -o generate_textures.exe
// Run: ./generate_textures.exe

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <filesystem>
#include <fstream>

// ============================================================
// Dino drawing utilities - creates detailed pixel-art dinos
// ============================================================

struct DinoPose {
    std::string name;
    // Leg positions: tuck, stride_left, stride_right
    float legOffset1;
    float legOffset2;
    float headTilt;
    float bodySquash;
    bool armsUp;
};

// Four animation poses
static const DinoPose POSES[] = {
    {"run1",   8.0f, -6.0f, 0.0f, 1.0f, false},  // Running stride 1
    {"run2",  -6.0f,  8.0f, 0.0f, 1.0f, false},  // Running stride 2
    {"jump",   0.0f,  0.0f, 2.0f, 0.85f, true},   // Jumping - legs tucked
    {"crouch", 4.0f,  4.0f, -3.0f, 0.6f, false},  // Crouching
};

// Draw a T-Rex style dino at given size with specific pose
sf::Image drawTrex(int frameW, int frameH, const DinoPose& pose, sf::Color bodyColor) {
    sf::RenderTexture rt;
    rt.create(frameW, frameH);
    rt.clear(sf::Color::Transparent);

    float w = static_cast<float>(frameW);
    float h = static_cast<float>(frameH);
    float cx = w * 0.45f;  // center x of body
    float cy = h * 0.65f;  // ground level (bottom of feet)

    // Scale factors based on frame size
    float scale = w / 128.0f;
    float headS = 1.0f + pose.headTilt * 0.01f;
    float bodyS = pose.bodySquash;

    sf::Color darkColor(
        std::max(0, static_cast<int>(bodyColor.r) - 40),
        std::max(0, static_cast<int>(bodyColor.g) - 40),
        std::max(0, static_cast<int>(bodyColor.b) - 40)
    );
    sf::Color lightColor(
        std::min(255, static_cast<int>(bodyColor.r) + 30),
        std::min(255, static_cast<int>(bodyColor.g) + 30),
        std::min(255, static_cast<int>(bodyColor.b) + 30)
    );

    // === TAIL ===
    sf::ConvexShape tail;
    tail.setPointCount(5);
    tail.setPoint(0, sf::Vector2f(cx - 28*scale*bodyS, cy - 22*scale));
    tail.setPoint(1, sf::Vector2f(cx - 42*scale*bodyS, cy - 14*scale));
    tail.setPoint(2, sf::Vector2f(cx - 48*scale*bodyS, cy - 8*scale));
    tail.setPoint(3, sf::Vector2f(cx - 38*scale*bodyS, cy - 12*scale));
    tail.setPoint(4, sf::Vector2f(cx - 25*scale*bodyS, cy - 18*scale));
    tail.setFillColor(darkColor);
    tail.setOutlineColor(darkColor);
    tail.setOutlineThickness(0.5f);
    rt.draw(tail);

    // === BODY (main torso) ===
    sf::ConvexShape body;
    body.setPointCount(8);
    float bw = 32 * scale * bodyS;  // body width
    float bh = 28 * scale;          // body height
    float bx = cx - bw * 0.35f;
    float by = cy - bh - 2*scale * bodyS;
    body.setPoint(0, sf::Vector2f(bx, by));
    body.setPoint(1, sf::Vector2f(bx + bw*0.7f, by - 2*scale*headS));
    body.setPoint(2, sf::Vector2f(bx + bw*1.0f, by + 4*scale*headS));
    body.setPoint(3, sf::Vector2f(bx + bw*1.1f, by + bh*0.4f));
    body.setPoint(4, sf::Vector2f(bx + bw*1.0f, by + bh*0.7f));
    body.setPoint(5, sf::Vector2f(bx + bw*0.8f, by + bh*0.9f));
    body.setPoint(6, sf::Vector2f(bx + bw*0.3f, by + bh*1.0f));
    body.setPoint(7, sf::Vector2f(bx - bw*0.1f, by + bh*0.6f));
    body.setFillColor(bodyColor);
    body.setOutlineColor(darkColor);
    body.setOutlineThickness(0.5f);
    rt.draw(body);

    // === HEAD ===
    sf::ConvexShape head;
    head.setPointCount(6);
    float hx = bx + bw * 0.85f;
    float hy = by - 3*scale * headS;
    float hw = 18 * scale * headS;
    float hh = 16 * scale;
    head.setPoint(0, sf::Vector2f(hx, hy));
    head.setPoint(1, sf::Vector2f(hx + hw, hy));
    head.setPoint(2, sf::Vector2f(hx + hw*1.2f, hy + hh*0.3f));
    head.setPoint(3, sf::Vector2f(hx + hw*1.15f, hy + hh*0.7f));
    head.setPoint(4, sf::Vector2f(hx + hw*0.8f, hy + hh*1.0f));
    head.setPoint(5, sf::Vector2f(hx, hy + hh*0.8f));
    head.setFillColor(bodyColor);
    head.setOutlineColor(darkColor);
    head.setOutlineThickness(0.5f);
    rt.draw(head);

    // === JAW (lower) ===
    sf::ConvexShape jaw;
    jaw.setPointCount(4);
    jaw.setPoint(0, sf::Vector2f(hx + hw*0.2f, hy + hh*0.65f));
    jaw.setPoint(1, sf::Vector2f(hx + hw*1.0f, hy + hh*0.6f));
    jaw.setPoint(2, sf::Vector2f(hx + hw*1.1f, hy + hh*0.9f));
    jaw.setPoint(3, sf::Vector2f(hx + hw*0.3f, hy + hh*0.95f));
    jaw.setFillColor(darkColor);
    jaw.setOutlineColor(sf::Color::Black);
    jaw.setOutlineThickness(0.3f);
    rt.draw(jaw);

    // === EYE ===
    float eyeX = hx + hw * 0.6f;
    float eyeY = hy + hh * 0.25f;
    float eyeR = 2.5f * scale;
    sf::CircleShape eye(eyeR * headS);
    eye.setFillColor(sf::Color::White);
    eye.setPosition(eyeX - eyeR, eyeY - eyeR);
    rt.draw(eye);
    sf::CircleShape pupil(eyeR * 0.6f * headS);
    pupil.setFillColor(sf::Color::Black);
    pupil.setPosition(eyeX - eyeR*0.6f, eyeY - eyeR*0.6f);
    rt.draw(pupil);

    // === SMALL ARMS ===
    float armX = bx + bw * 0.5f;
    float armY = by + bh * 0.35f;
    float armLen = 8 * scale;
    float armW = 3 * scale;
    if (pose.armsUp) {
        // Arms up in jump
        sf::RectangleShape arm(sf::Vector2f(armW, armLen));
        arm.setFillColor(darkColor);
        arm.setPosition(armX, armY - armLen);
        arm.setRotation(-30);
        rt.draw(arm);
        sf::RectangleShape arm2(sf::Vector2f(armW, armLen * 0.8f));
        arm2.setFillColor(darkColor);
        arm2.setPosition(armX + 5*scale, armY - armLen*0.7f);
        arm2.setRotation(-45);
        rt.draw(arm2);
    } else {
        sf::RectangleShape arm(sf::Vector2f(armW, armLen));
        arm.setFillColor(darkColor);
        arm.setPosition(armX, armY);
        arm.setRotation(10);
        rt.draw(arm);
        sf::RectangleShape arm2(sf::Vector2f(armW, armLen*0.8f));
        arm2.setFillColor(darkColor);
        arm2.setPosition(armX + 6*scale, armY + 2*scale);
        arm2.setRotation(20);
        rt.draw(arm2);
    }

    // === LEGS ===
    float legY = cy;
    float legH = 18 * scale * bodyS;
    float legW = 4 * scale;
    float lx1 = bx + bw * 0.3f + pose.legOffset1 * scale * 0.5f;
    float lx2 = bx + bw * 0.6f + pose.legOffset2 * scale * 0.5f;

    // Leg 1
    sf::ConvexShape leg1;
    leg1.setPointCount(4);
    leg1.setPoint(0, sf::Vector2f(lx1 - legW*0.5f, legY - legH));
    leg1.setPoint(1, sf::Vector2f(lx1 + legW*0.5f, legY - legH*0.9f));
    leg1.setPoint(2, sf::Vector2f(lx1 + legW*0.8f, legY));
    leg1.setPoint(3, sf::Vector2f(lx1 - legW*0.3f, legY));
    leg1.setFillColor(darkColor);
    leg1.setOutlineColor(sf::Color::Black);
    leg1.setOutlineThickness(0.3f);
    rt.draw(leg1);

    // Leg 2
    sf::ConvexShape leg2;
    leg2.setPointCount(4);
    leg2.setPoint(0, sf::Vector2f(lx2 - legW*0.5f, legY - legH*0.95f));
    leg2.setPoint(1, sf::Vector2f(lx2 + legW*0.5f, legY - legH*0.85f));
    leg2.setPoint(2, sf::Vector2f(lx2 + legW*0.8f, legY));
    leg2.setPoint(3, sf::Vector2f(lx2 - legW*0.3f, legY));
    leg2.setFillColor(bodyColor);
    leg2.setOutlineColor(sf::Color::Black);
    leg2.setOutlineThickness(0.3f);
    rt.draw(leg2);

    // === FEET ===
    float footH = 3 * scale;
    float footW = 7 * scale;
    sf::RectangleShape foot1(sf::Vector2f(footW, footH));
    foot1.setFillColor(darkColor);
    foot1.setPosition(lx1 - footW*0.3f, legY - footH);
    rt.draw(foot1);
    sf::RectangleShape foot2(sf::Vector2f(footW, footH));
    foot2.setFillColor(darkColor);
    foot2.setPosition(lx2 - footW*0.3f, legY - footH);
    rt.draw(foot2);

    // === BELLY HIGHLIGHT ===
    sf::ConvexShape belly;
    belly.setPointCount(5);
    belly.setPoint(0, sf::Vector2f(bx + bw*0.2f, by + bh*0.6f));
    belly.setPoint(1, sf::Vector2f(bx + bw*0.5f, by + bh*0.55f));
    belly.setPoint(2, sf::Vector2f(bx + bw*0.8f, by + bh*0.6f));
    belly.setPoint(3, sf::Vector2f(bx + bw*0.7f, by + bh*0.85f));
    belly.setPoint(4, sf::Vector2f(bx + bw*0.3f, by + bh*0.85f));
    belly.setFillColor(sf::Color(
        std::min(255, bodyColor.r + 40),
        std::min(255, bodyColor.g + 40),
        std::min(255, bodyColor.b + 40)
    ));
    rt.draw(belly);

    rt.display();
    return rt.getTexture().copyToImage();
}

// Draw a Raptor style dino
sf::Image drawRaptor(int frameW, int frameH, const DinoPose& pose, sf::Color bodyColor) {
    sf::RenderTexture rt;
    rt.create(frameW, frameH);
    rt.clear(sf::Color::Transparent);

    float w = static_cast<float>(frameW);
    float h = static_cast<float>(frameH);
    float cx = w * 0.45f;
    float cy = h * 0.65f;
    float scale = w / 128.0f;
    float headS = 1.0f + pose.headTilt * 0.01f;
    float bodyS = pose.bodySquash;

    sf::Color darkColor(
        std::max(0, static_cast<int>(bodyColor.r) - 35),
        std::max(0, static_cast<int>(bodyColor.g) - 35),
        std::max(0, static_cast<int>(bodyColor.b) - 35)
    );
    sf::Color lightColor(
        std::min(255, static_cast<int>(bodyColor.r) + 35),
        std::min(255, static_cast<int>(bodyColor.g) + 35),
        std::min(255, static_cast<int>(bodyColor.b) + 35)
    );

    // === TAIL (longer, thinner - raptor style) ===
    sf::ConvexShape tail;
    tail.setPointCount(5);
    tail.setPoint(0, sf::Vector2f(cx - 18*scale*bodyS, cy - 24*scale));
    tail.setPoint(1, sf::Vector2f(cx - 50*scale*bodyS, cy - 12*scale));
    tail.setPoint(2, sf::Vector2f(cx - 55*scale*bodyS, cy - 6*scale));
    tail.setPoint(3, sf::Vector2f(cx - 40*scale*bodyS, cy - 8*scale));
    tail.setPoint(4, sf::Vector2f(cx - 15*scale*bodyS, cy - 18*scale));
    tail.setFillColor(darkColor);
    rt.draw(tail);

    // === BODY (sleeker, more aerodynamic) ===
    sf::ConvexShape body;
    body.setPointCount(8);
    float bw = 28 * scale * bodyS;
    float bh = 26 * scale;
    float bx = cx - bw * 0.3f;
    float by = cy - bh - 2*scale * bodyS;
    body.setPoint(0, sf::Vector2f(bx, by + 2*scale));
    body.setPoint(1, sf::Vector2f(bx + bw*0.6f, by));
    body.setPoint(2, sf::Vector2f(bx + bw*1.0f, by + 3*scale*headS));
    body.setPoint(3, sf::Vector2f(bx + bw*1.15f, by + bh*0.35f));
    body.setPoint(4, sf::Vector2f(bx + bw*1.1f, by + bh*0.65f));
    body.setPoint(5, sf::Vector2f(bx + bw*0.85f, by + bh*0.85f));
    body.setPoint(6, sf::Vector2f(bx + bw*0.35f, by + bh*0.95f));
    body.setPoint(7, sf::Vector2f(bx - bw*0.1f, by + bh*0.6f));
    body.setFillColor(bodyColor);
    body.setOutlineColor(darkColor);
    body.setOutlineThickness(0.5f);
    rt.draw(body);

    // === HEAD (more pointed, raptor-like) ===
    sf::ConvexShape head;
    head.setPointCount(6);
    float hx = bx + bw * 0.8f;
    float hy = by - 2*scale * headS;
    float hw = 20 * scale * headS;
    float hh = 14 * scale;
    head.setPoint(0, sf::Vector2f(hx, hy + 2*scale));
    head.setPoint(1, sf::Vector2f(hx + hw, hy));
    head.setPoint(2, sf::Vector2f(hx + hw*1.35f, hy + hh*0.25f));
    head.setPoint(3, sf::Vector2f(hx + hw*1.3f, hy + hh*0.6f));
    head.setPoint(4, sf::Vector2f(hx + hw*0.9f, hy + hh*0.9f));
    head.setPoint(5, sf::Vector2f(hx, hy + hh*0.7f));
    head.setFillColor(bodyColor);
    head.setOutlineColor(darkColor);
    head.setOutlineThickness(0.5f);
    rt.draw(head);

    // === CLAW ON FOOT (raptor's signature) ===
    float clawX = cx + bw * 0.3f + pose.legOffset1 * scale * 0.3f;
    sf::ConvexShape claw;
    claw.setPointCount(3);
    claw.setPoint(0, sf::Vector2f(clawX - 2*scale, cy - 3*scale));
    claw.setPoint(1, sf::Vector2f(clawX + 2*scale, cy - 1*scale));
    claw.setPoint(2, sf::Vector2f(clawX, cy - 7*scale));
    claw.setFillColor(sf::Color(80, 60, 40));

    // === EYE (more menacing) ===
    float eyeX = hx + hw * 0.55f;
    float eyeY = hy + hh * 0.2f;
    float eyeR = 2.5f * scale;
    sf::CircleShape eye(eyeR * headS);
    eye.setFillColor(sf::Color(255, 200, 50));
    eye.setPosition(eyeX - eyeR, eyeY - eyeR);
    rt.draw(eye);
    sf::CircleShape pupil(eyeR * 0.65f * headS);
    pupil.setFillColor(sf::Color::Black);
    pupil.setPosition(eyeX - eyeR*0.65f, eyeY - eyeR*0.65f);
    rt.draw(pupil);

    // === LEGS (more muscular) ===
    float legY = cy;
    float legH = 20 * scale * bodyS;
    float legW = 4.5f * scale;
    float lx1 = bx + bw * 0.3f + pose.legOffset1 * scale * 0.4f;
    float lx2 = bx + bw * 0.6f + pose.legOffset2 * scale * 0.4f;

    sf::ConvexShape leg1;
    leg1.setPointCount(4);
    leg1.setPoint(0, sf::Vector2f(lx1 - legW*0.5f, legY - legH));
    leg1.setPoint(1, sf::Vector2f(lx1 + legW*0.5f, legY - legH*0.9f));
    leg1.setPoint(2, sf::Vector2f(lx1 + legW*0.7f, legY));
    leg1.setPoint(3, sf::Vector2f(lx1 - legW*0.3f, legY));
    leg1.setFillColor(darkColor);
    rt.draw(leg1);

    sf::ConvexShape leg2;
    leg2.setPointCount(4);
    leg2.setPoint(0, sf::Vector2f(lx2 - legW*0.5f, legY - legH*0.95f));
    leg2.setPoint(1, sf::Vector2f(lx2 + legW*0.5f, legY - legH*0.85f));
    leg2.setPoint(2, sf::Vector2f(lx2 + legW*0.7f, legY));
    leg2.setPoint(3, sf::Vector2f(lx2 - legW*0.3f, legY));
    leg2.setFillColor(bodyColor);
    rt.draw(leg2);

    // === FEATHERS (raptor had feathers!) ===
    sf::ConvexShape feather1;
    feather1.setPointCount(3);
    feather1.setPoint(0, sf::Vector2f(bx + bw*0.5f, by + 4*scale));
    feather1.setPoint(1, sf::Vector2f(bx + bw*0.6f, by - 4*scale));
    feather1.setPoint(2, sf::Vector2f(bx + bw*0.4f, by - 2*scale));
    feather1.setFillColor(lightColor);
    rt.draw(feather1);

    sf::ConvexShape feather2;
    feather2.setPointCount(3);
    feather2.setPoint(0, sf::Vector2f(bx + bw*0.6f, by + 6*scale));
    feather2.setPoint(1, sf::Vector2f(bx + bw*0.7f, by - 2*scale));
    feather2.setPoint(2, sf::Vector2f(bx + bw*0.5f, by));
    feather2.setFillColor(bodyColor);
    rt.draw(feather2);

    rt.display();
    return rt.getTexture().copyToImage();
}

// Draw a Triceratops style dino
sf::Image drawTriceratops(int frameW, int frameH, const DinoPose& pose, sf::Color bodyColor) {
    sf::RenderTexture rt;
    rt.create(frameW, frameH);
    rt.clear(sf::Color::Transparent);

    float w = static_cast<float>(frameW);
    float h = static_cast<float>(frameH);
    float cx = w * 0.45f;
    float cy = h * 0.65f;
    float scale = w / 128.0f;
    float headS = 1.0f + pose.headTilt * 0.01f;
    float bodyS = pose.bodySquash;

    sf::Color darkColor(
        std::max(0, static_cast<int>(bodyColor.r) - 40),
        std::max(0, static_cast<int>(bodyColor.g) - 40),
        std::max(0, static_cast<int>(bodyColor.b) - 40)
    );
    sf::Color lightColor(
        std::min(255, static_cast<int>(bodyColor.r) + 30),
        std::min(255, static_cast<int>(bodyColor.g) + 30),
        std::min(255, static_cast<int>(bodyColor.b) + 30)
    );

    // === TAIL (shorter, wider) ===
    sf::ConvexShape tail;
    tail.setPointCount(5);
    tail.setPoint(0, sf::Vector2f(cx - 30*scale*bodyS, cy - 24*scale));
    tail.setPoint(1, sf::Vector2f(cx - 38*scale*bodyS, cy - 18*scale));
    tail.setPoint(2, sf::Vector2f(cx - 40*scale*bodyS, cy - 10*scale));
    tail.setPoint(3, sf::Vector2f(cx - 30*scale*bodyS, cy - 14*scale));
    tail.setPoint(4, sf::Vector2f(cx - 20*scale*bodyS, cy - 20*scale));
    tail.setFillColor(darkColor);
    rt.draw(tail);

    // === BODY (bulkier, wider) ===
    sf::ConvexShape body;
    body.setPointCount(8);
    float bw = 38 * scale * bodyS;
    float bh = 28 * scale;
    float bx = cx - bw * 0.35f;
    float by = cy - bh - 2*scale * bodyS;
    body.setPoint(0, sf::Vector2f(bx, by));
    body.setPoint(1, sf::Vector2f(bx + bw*0.65f, by - 2*scale*headS));
    body.setPoint(2, sf::Vector2f(bx + bw*0.95f, by + 2*scale*headS));
    body.setPoint(3, sf::Vector2f(bx + bw*1.05f, by + bh*0.35f));
    body.setPoint(4, sf::Vector2f(bx + bw*1.0f, by + bh*0.7f));
    body.setPoint(5, sf::Vector2f(bx + bw*0.8f, by + bh*0.9f));
    body.setPoint(6, sf::Vector2f(bx + bw*0.25f, by + bh*1.0f));
    body.setPoint(7, sf::Vector2f(bx - bw*0.15f, by + bh*0.65f));
    body.setFillColor(bodyColor);
    body.setOutlineColor(darkColor);
    body.setOutlineThickness(0.5f);
    rt.draw(body);

    // === FRIL (the iconic neck frill) ===
    sf::ConvexShape frill;
    frill.setPointCount(6);
    float frx = bx + bw * 0.55f;
    float fry = by - 4*scale * headS;
    frill.setPoint(0, sf::Vector2f(frx, fry));
    frill.setPoint(1, sf::Vector2f(frx + 20*scale*headS, fry - 6*scale));
    frill.setPoint(2, sf::Vector2f(frx + 30*scale*headS, fry + 2*scale));
    frill.setPoint(3, sf::Vector2f(frx + 28*scale*headS, fry + 10*scale));
    frill.setPoint(4, sf::Vector2f(frx + 18*scale*headS, fry + 12*scale));
    frill.setPoint(5, sf::Vector2f(frx + 5*scale, fry + 6*scale));
    frill.setFillColor(lightColor);
    frill.setOutlineColor(darkColor);
    frill.setOutlineThickness(1.0f);
    rt.draw(frill);

    // Frill border dots
    for (int i = 0; i < 5; i++) {
        sf::CircleShape dot(1.5f * scale);
        dot.setFillColor(sf::Color(80, 60, 40));
        float angle = 0.3f + i * 0.25f;
        dot.setPosition(frx + 15*scale + std::cos(angle) * 12*scale,
                        fry - 2*scale + std::sin(angle) * 7*scale);
        rt.draw(dot);
    }

    // === HEAD (wider, with beak) ===
    sf::ConvexShape head;
    head.setPointCount(6);
    float hx = frx + 15*scale;
    float hy = fry + 8*scale;
    float hw = 18 * scale * headS;
    float hh = 16 * scale;
    head.setPoint(0, sf::Vector2f(hx, hy));
    head.setPoint(1, sf::Vector2f(hx + hw, hy));
    head.setPoint(2, sf::Vector2f(hx + hw*1.2f, hy + hh*0.3f));
    head.setPoint(3, sf::Vector2f(hx + hw*1.15f, hy + hh*0.65f));
    head.setPoint(4, sf::Vector2f(hx + hw*0.8f, hy + hh*0.95f));
    head.setPoint(5, sf::Vector2f(hx, hy + hh*0.75f));
    head.setFillColor(bodyColor);
    head.setOutlineColor(darkColor);
    head.setOutlineThickness(0.5f);
    rt.draw(head);

    // === BEAK (parrot-like) ===
    sf::ConvexShape beak;
    beak.setPointCount(3);
    beak.setPoint(0, sf::Vector2f(hx + hw*0.8f, hy + hh*0.4f));
    beak.setPoint(1, sf::Vector2f(hx + hw*1.35f, hy + hh*0.45f));
    beak.setPoint(2, sf::Vector2f(hx + hw*0.85f, hy + hh*0.7f));
    beak.setFillColor(sf::Color(80, 60, 40));
    rt.draw(beak);

    // === EYE ===
    float eyeX = hx + hw * 0.55f;
    float eyeY = hy + hh * 0.2f;
    float eyeR = 2.5f * scale;
    sf::CircleShape eye(eyeR * headS);
    eye.setFillColor(sf::Color::White);
    eye.setPosition(eyeX - eyeR, eyeY - eyeR);
    rt.draw(eye);
    sf::CircleShape pupil(eyeR * 0.6f * headS);
    pupil.setFillColor(sf::Color::Black);
    pupil.setPosition(eyeX - eyeR*0.6f, eyeY - eyeR*0.6f);
    rt.draw(pupil);

    // === HORNS ===
    sf::ConvexShape horn1;
    horn1.setPointCount(3);
    horn1.setPoint(0, sf::Vector2f(eyeX, eyeY - 4*scale));
    horn1.setPoint(1, sf::Vector2f(eyeX + 3*scale, eyeY - 10*scale));
    horn1.setPoint(2, sf::Vector2f(eyeX - 1*scale, eyeY - 6*scale));
    horn1.setFillColor(sf::Color(180, 160, 120));
    rt.draw(horn1);

    sf::ConvexShape horn2;
    horn2.setPointCount(3);
    horn2.setPoint(0, sf::Vector2f(frx + 12*scale, fry + 4*scale));
    horn2.setPoint(1, sf::Vector2f(frx + 8*scale, fry - 6*scale));
    horn2.setPoint(2, sf::Vector2f(frx + 5*scale, fry));
    horn2.setFillColor(sf::Color(160, 140, 100));
    rt.draw(horn2);

    // === LEGS (thicker, elephant-like) ===
    float legY = cy;
    float legH = 16 * scale * bodyS;
    float legW = 6 * scale;
    float lx1 = bx + bw * 0.25f + pose.legOffset1 * scale * 0.3f;
    float lx2 = bx + bw * 0.6f + pose.legOffset2 * scale * 0.3f;

    sf::ConvexShape leg1;
    leg1.setPointCount(4);
    leg1.setPoint(0, sf::Vector2f(lx1 - legW*0.5f, legY - legH));
    leg1.setPoint(1, sf::Vector2f(lx1 + legW*0.5f, legY - legH*0.9f));
    leg1.setPoint(2, sf::Vector2f(lx1 + legW*0.9f, legY));
    leg1.setPoint(3, sf::Vector2f(lx1 - legW*0.4f, legY));
    leg1.setFillColor(darkColor);
    leg1.setOutlineColor(sf::Color::Black);
    leg1.setOutlineThickness(0.5f);
    rt.draw(leg1);

    sf::ConvexShape leg2;
    leg2.setPointCount(4);
    leg2.setPoint(0, sf::Vector2f(lx2 - legW*0.5f, legY - legH*0.95f));
    leg2.setPoint(1, sf::Vector2f(lx2 + legW*0.5f, legY - legH*0.85f));
    leg2.setPoint(2, sf::Vector2f(lx2 + legW*0.9f, legY));
    leg2.setPoint(3, sf::Vector2f(lx2 - legW*0.4f, legY));
    leg2.setFillColor(bodyColor);
    leg2.setOutlineColor(sf::Color::Black);
    leg2.setOutlineThickness(0.5f);
    rt.draw(leg2);

    rt.display();
    return rt.getTexture().copyToImage();
}

// ============================================================
// Obstacle drawing
// ============================================================

// Draw a single cactus frame with optional lean offset
sf::Image drawCactusFrame(int w, int h, float lean) {
    sf::RenderTexture rt;
    rt.create(w, h);
    rt.clear(sf::Color::Transparent);

    float s = w / static_cast<float>(w < 40 ? 30 : 45);
    bool isSmall = (w < 40);

    sf::Color green(50, 140, 50);
    sf::Color darkGreen(35, 100, 35);
    float trunkW = isSmall ? 10.0f : 14.0f;
    float trunkH = isSmall ? 50.0f : 70.0f;

    // Main trunk
    sf::RectangleShape trunk(sf::Vector2f(trunkW, trunkH));
    trunk.setFillColor(green);
    trunk.setPosition((isSmall ? 10 : 15) + lean, 0);
    rt.draw(trunk);

    // Left arms
    sf::RectangleShape lArm(sf::Vector2f(trunkH * 0.3f, trunkW * 0.6f));
    lArm.setFillColor(green);
    lArm.setPosition((isSmall ? 0 : 5) + lean * 0.5f, isSmall ? 15 : 20);
    rt.draw(lArm);

    // Right arms
    sf::RectangleShape rArm(sf::Vector2f(trunkH * 0.25f, trunkW * 0.6f));
    rArm.setFillColor(green);
    rArm.setPosition((isSmall ? 20 : 30) + lean * 0.5f, isSmall ? 10 : 15);
    rt.draw(rArm);

    // Spine details
    int numSpines = isSmall ? 4 : 6;
    float spineSpacing = isSmall ? 12.0f : 11.0f;
    for (int i = 0; i < numSpines; i++) {
        sf::ConvexShape spine;
        spine.setPointCount(3);
        float sx = (isSmall ? 12 : 17);
        spine.setPoint(0, sf::Vector2f(sx + lean, 8*s + i*spineSpacing*s));
        spine.setPoint(1, sf::Vector2f(sx + 5*s + lean, 6*s + i*spineSpacing*s));
        spine.setPoint(2, sf::Vector2f(sx + 3*s + lean, 10*s + i*spineSpacing*s));
        spine.setFillColor(darkGreen);
        rt.draw(spine);
    }

    // Trunk detail lines
    sf::RectangleShape detail(sf::Vector2f(1.5f, trunkH * 0.4f));
    detail.setFillColor(darkGreen);
    detail.setPosition((isSmall ? 13 : 19) + lean, trunkH * 0.1f);
    rt.draw(detail);
    sf::RectangleShape detail2(sf::Vector2f(1.5f, trunkH * 0.35f));
    detail2.setFillColor(darkGreen);
    detail2.setPosition((isSmall ? 16 : 24) + lean, trunkH * 0.15f);
    rt.draw(detail2);

    rt.display();
    return rt.getTexture().copyToImage();
}

// Generate cactus sprite sheet (2 frames: sway left, sway right)
sf::Image drawCactusSpriteSheet(int fw, int fh) {
    sf::Image sheet;
    sheet.create(fw * 2, fh, sf::Color::Transparent);
    sheet.copy(drawCactusFrame(fw, fh, -1.5f), 0, 0);
    sheet.copy(drawCactusFrame(fw, fh, 1.5f), fw, 0);
    return sheet;
}

// Draw a pterodactyl frame with optional wing position
sf::Image drawPterodactylFrame(int w, int h, bool wingsUp) {
    sf::RenderTexture rt;
    rt.create(w, h);
    rt.clear(sf::Color::Transparent);
    float s = w / 60.0f;

    sf::Color brown(140, 90, 55);
    sf::Color darkBrown(100, 65, 35);

    // Body
    sf::CircleShape bodyCircle(12*s);
    bodyCircle.setScale(1.5f, 0.8f);
    bodyCircle.setFillColor(brown);
    bodyCircle.setPosition(8*s, 12*s);
    rt.draw(bodyCircle);

    // Wing
    sf::ConvexShape wing;
    if (wingsUp) {
        wing.setPointCount(4);
        wing.setPoint(0, sf::Vector2f(15*s, 18*s));
        wing.setPoint(1, sf::Vector2f(22*s, 2*s));
        wing.setPoint(2, sf::Vector2f(50*s, 8*s));
        wing.setPoint(3, sf::Vector2f(42*s, 20*s));
    } else {
        wing.setPointCount(4);
        wing.setPoint(0, sf::Vector2f(15*s, 18*s));
        wing.setPoint(1, sf::Vector2f(20*s, 28*s));
        wing.setPoint(2, sf::Vector2f(48*s, 32*s));
        wing.setPoint(3, sf::Vector2f(40*s, 22*s));
    }
    wing.setFillColor(brown);
    wing.setOutlineColor(darkBrown);
    wing.setOutlineThickness(0.5f);
    rt.draw(wing);

    // Wing membrane
    sf::ConvexShape membrane;
    if (wingsUp) {
        membrane.setPointCount(4);
        membrane.setPoint(0, sf::Vector2f(20*s, 14*s));
        membrane.setPoint(1, sf::Vector2f(45*s, 10*s));
        membrane.setPoint(2, sf::Vector2f(30*s, 16*s));
        membrane.setPoint(3, sf::Vector2f(18*s, 10*s));
    } else {
        membrane.setPointCount(4);
        membrane.setPoint(0, sf::Vector2f(18*s, 22*s));
        membrane.setPoint(1, sf::Vector2f(45*s, 28*s));
        membrane.setPoint(2, sf::Vector2f(30*s, 24*s));
        membrane.setPoint(3, sf::Vector2f(18*s, 20*s));
    }
    membrane.setFillColor(sf::Color(170, 115, 75, 140));
    rt.draw(membrane);

    // Head/neck
    sf::ConvexShape head;
    head.setPointCount(4);
    head.setPoint(0, sf::Vector2f(42*s, 10*s));
    head.setPoint(1, sf::Vector2f(56*s, 12*s));
    head.setPoint(2, sf::Vector2f(54*s, 20*s));
    head.setPoint(3, sf::Vector2f(40*s, 20*s));
    head.setFillColor(brown);
    rt.draw(head);

    // Beak
    sf::ConvexShape beak;
    beak.setPointCount(3);
    beak.setPoint(0, sf::Vector2f(55*s, 13*s));
    beak.setPoint(1, sf::Vector2f(60*s, 16*s));
    beak.setPoint(2, sf::Vector2f(55*s, 19*s));
    beak.setFillColor(sf::Color(180, 130, 60));
    rt.draw(beak);

    // Eye
    sf::CircleShape eye(1.5f*s);
    eye.setFillColor(sf::Color::White);
    eye.setPosition(44*s, 12*s);
    rt.draw(eye);
    sf::CircleShape pupil(0.8f*s);
    pupil.setFillColor(sf::Color::Black);
    pupil.setPosition(45*s, 13*s);
    rt.draw(pupil);

    // Tail
    sf::ConvexShape tail;
    tail.setPointCount(3);
    tail.setPoint(0, sf::Vector2f(8*s, 16*s));
    tail.setPoint(1, sf::Vector2f(0, 22*s));
    tail.setPoint(2, sf::Vector2f(6*s, 24*s));
    tail.setFillColor(darkBrown);
    rt.draw(tail);

    rt.display();
    return rt.getTexture().copyToImage();
}

// Generate pterodactyl sprite sheet (2 frames: wings up, wings down)
sf::Image drawPterodactylSpriteSheet(int fw, int fh) {
    sf::Image sheet;
    sheet.create(fw * 2, fh, sf::Color::Transparent);
    sheet.copy(drawPterodactylFrame(fw, fh, true), 0, 0);   // wings up
    sheet.copy(drawPterodactylFrame(fw, fh, false), fw, 0); // wings down
    return sheet;
}

sf::Image drawRock(int w, int h) {
    sf::RenderTexture rt;
    rt.create(w, h);
    rt.clear(sf::Color::Transparent);
    float s = w / 50.0f;

    sf::Color gray(120, 110, 100);
    sf::Color darkGray(90, 80, 70);
    sf::Color lightGray(155, 145, 135);

    // Main rock body
    sf::ConvexShape rock;
    rock.setPointCount(10);
    rock.setPoint(0, sf::Vector2f(5*s, 38*s));
    rock.setPoint(1, sf::Vector2f(2*s, 25*s));
    rock.setPoint(2, sf::Vector2f(4*s, 12*s));
    rock.setPoint(3, sf::Vector2f(10*s, 5*s));
    rock.setPoint(4, sf::Vector2f(20*s, 3*s));
    rock.setPoint(5, sf::Vector2f(32*s, 4*s));
    rock.setPoint(6, sf::Vector2f(42*s, 8*s));
    rock.setPoint(7, sf::Vector2f(48*s, 18*s));
    rock.setPoint(8, sf::Vector2f(47*s, 30*s));
    rock.setPoint(9, sf::Vector2f(42*s, 38*s));
    rock.setFillColor(gray);
    rock.setOutlineColor(darkGray);
    rock.setOutlineThickness(0.5f);
    rt.draw(rock);

    // Highlight/cracks
    sf::ConvexShape highlight;
    highlight.setPointCount(5);
    highlight.setPoint(0, sf::Vector2f(15*s, 10*s));
    highlight.setPoint(1, sf::Vector2f(25*s, 6*s));
    highlight.setPoint(2, sf::Vector2f(35*s, 10*s));
    highlight.setPoint(3, sf::Vector2f(28*s, 16*s));
    highlight.setPoint(4, sf::Vector2f(18*s, 18*s));
    highlight.setFillColor(lightGray);
    rt.draw(highlight);

    // Crack line
    sf::ConvexShape crack;
    crack.setPointCount(3);
    crack.setPoint(0, sf::Vector2f(30*s, 8*s));
    crack.setPoint(1, sf::Vector2f(25*s, 20*s));
    crack.setPoint(2, sf::Vector2f(35*s, 12*s));
    crack.setFillColor(darkGray);
    rt.draw(crack);

    rt.display();
    return rt.getTexture().copyToImage();
}

sf::Image drawCoin(int w, int h) {
    sf::RenderTexture rt;
    rt.create(w, h);
    rt.clear(sf::Color::Transparent);
    float s = w / 24.0f;

    // Outer ring
    sf::CircleShape outer(11*s);
    outer.setFillColor(sf::Color(255, 215, 0));
    outer.setOutlineColor(sf::Color(200, 170, 0));
    outer.setOutlineThickness(2*s);
    outer.setPosition(1*s, 1*s);
    rt.draw(outer);

    // Inner circle
    sf::CircleShape inner(7*s);
    inner.setFillColor(sf::Color(255, 235, 100));
    inner.setPosition(5*s, 5*s);
    rt.draw(inner);

    // Star symbol
    sf::ConvexShape star;
    star.setPointCount(10);
    float starCx = w/2.0f, starCy = h/2.0f;
    for (int i = 0; i < 10; i++) {
        float angle = i * 3.14159f / 5.0f - 3.14159f / 2.0f;
        float r = (i % 2 == 0) ? 5*s : 2.5f*s;
        star.setPoint(i, sf::Vector2f(starCx + std::cos(angle) * r, starCy + std::sin(angle) * r));
    }
    star.setFillColor(sf::Color(200, 170, 0));
    rt.draw(star);

    // Shine
    sf::CircleShape shine(2*s);
    shine.setFillColor(sf::Color(255, 255, 255, 100));
    shine.setPosition(5*s, 4*s);
    rt.draw(shine);

    rt.display();
    return rt.getTexture().copyToImage();
}

sf::Image drawCloud(int w, int h) {
    sf::RenderTexture rt;
    rt.create(w, h);
    rt.clear(sf::Color::Transparent);
    float s = w / 120.0f;

    sf::Color white(255, 255, 255);

    // Multiple overlapping puffs
    auto drawPuff = [&](float x, float y, float r, sf::Color c, sf::Uint8 alpha) {
        sf::CircleShape puff(r);
        puff.setFillColor(sf::Color(c.r, c.g, c.b, alpha));
        puff.setPosition(x - r, y - r);
        rt.draw(puff);
    };

    drawPuff(25*s, 28*s, 18*s, white, 180);
    drawPuff(50*s, 20*s, 22*s, white, 170);
    drawPuff(75*s, 25*s, 20*s, white, 160);
    drawPuff(95*s, 30*s, 16*s, white, 150);
    drawPuff(60*s, 35*s, 15*s, white, 140);
    drawPuff(35*s, 32*s, 14*s, white, 150);

    // Bottom fill to make flat bottom
    sf::RectangleShape bottom(sf::Vector2f(100*s, 10*s));
    bottom.setFillColor(sf::Color(255, 255, 255, 170));
    bottom.setPosition(10*s, 28*s);
    rt.draw(bottom);

    rt.display();
    return rt.getTexture().copyToImage();
}

sf::Image drawGroundTexture(int w, int h) {
    sf::RenderTexture rt;
    rt.create(w, h);
    rt.clear(sf::Color::Transparent);
    float s = w / 128.0f;

    // Dirt layer
    sf::RectangleShape ground(sf::Vector2f(w, 25*s));
    ground.setFillColor(sf::Color(160, 120, 70));
    ground.setPosition(0, 30*s);
    rt.draw(ground);

    // Top grass/dirt line
    sf::RectangleShape topLine(sf::Vector2f(w, 3*s));
    topLine.setFillColor(sf::Color(120, 90, 50));
    topLine.setPosition(0, 30*s);
    rt.draw(topLine);

    // Bottom darker layer
    sf::RectangleShape bottomLayer(sf::Vector2f(w, 10*s));
    bottomLayer.setFillColor(sf::Color(120, 90, 50));
    bottomLayer.setPosition(0, 50*s);
    rt.draw(bottomLayer);

    // Small rocks/pebbles
    srand(42);
    for (int i = 0; i < 8; i++) {
        float rs = 1.0f + (rand() % 3) * 0.5f * s;
        sf::CircleShape rock(rs);
        rock.setFillColor(sf::Color(140, 110, 80));
        rock.setPosition(5*s + i * 15*s + (rand() % 5)*s, 38*s + (rand() % 8)*s);
        rt.draw(rock);
    }

    // Grass tufts
    for (int i = 0; i < 6; i++) {
        sf::ConvexShape grass;
        grass.setPointCount(3);
        float gx = 10*s + i * 20*s;
        grass.setPoint(0, sf::Vector2f(gx, 30*s));
        grass.setPoint(1, sf::Vector2f(gx + 3*s, 24*s));
        grass.setPoint(2, sf::Vector2f(gx + 6*s, 30*s));
        grass.setFillColor(sf::Color(50, 140, 50));
        rt.draw(grass);
    }

    rt.display();
    return rt.getTexture().copyToImage();
}

sf::Image drawPowerUp(int w, int h, sf::Color color, const std::string& symbol) {
    sf::RenderTexture rt;
    rt.create(w, h);
    rt.clear(sf::Color::Transparent);
    float s = w / 32.0f;

    // Outer glow
    sf::CircleShape glow(16*s);
    glow.setFillColor(sf::Color(color.r, color.g, color.b, 60));
    glow.setPosition(0, 0);
    rt.draw(glow);

    // Main circle
    sf::CircleShape bg(13*s);
    bg.setFillColor(color);
    bg.setOutlineColor(sf::Color::White);
    bg.setOutlineThickness(2*s);
    bg.setPosition(3*s, 3*s);
    rt.draw(bg);

    // Inner circle
    sf::CircleShape inner(8*s);
    sf::Color lightColor = color;
    lightColor.r = std::min(255, color.r + 60);
    lightColor.g = std::min(255, color.g + 60);
    lightColor.b = std::min(255, color.b + 60);
    inner.setFillColor(lightColor);
    inner.setPosition(8*s, 8*s);
    rt.draw(inner);

    // Letter symbol
    if (!symbol.empty()) {
        // We'll draw a simple shape as symbol instead of text
        sf::CircleShape sym(3*s);
        sym.setFillColor(sf::Color::White);
        sym.setPosition(w/2 - 3*s, h/2 - 3*s);
        rt.draw(sym);
    }

    // Shine
    sf::CircleShape shine(3*s);
    shine.setFillColor(sf::Color(255, 255, 255, 120));
    shine.setPosition(5*s, 5*s);
    rt.draw(shine);

    rt.display();
    return rt.getTexture().copyToImage();
}

// ============================================================
// Main generator
// ============================================================

struct TextureSpec {
    std::string filename;
    std::string description;
    sf::Image (*generator)(int, int);
    int width;
    int height;
};

int main(int argc, char* argv[]) {
    std::string outDir = "resources";
    if (argc > 1) outDir = argv[1];

    std::filesystem::create_directories(outDir);

    std::cout << "╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "║   DinoRunner Professional Texture Gen    ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════╝" << std::endl;
    std::cout << "Output: " << outDir << "/" << std::endl << std::endl;

    int total = 0;

    // ============================================================
    // DINO SPRITE SHEET GENERATION
    // ============================================================
    struct DinoDef {
        std::string name;
        sf::Image (*drawFunc)(int, int, const DinoPose&, sf::Color);
        sf::Color color;
    };

    DinoDef dinos[] = {
        {"trex",        drawTrex,        sf::Color(80, 180, 80)},   // Green T-Rex
        {"raptor",      drawRaptor,      sf::Color(180, 130, 50)},  // Brown Raptor
        {"triceratops", drawTriceratops, sf::Color(100, 150, 200)}, // Blue Triceratops
    };

    // Size definitions: name, width, height
    struct SizeDef { std::string name; int w; int h; };
    SizeDef sizes[] = {
        {"32",  32,  32},
        {"64",  64,  64},
        {"96",  96,  96},
        {"128", 128, 72},
    };

    std::cout << "--- Generating Dino Sprite Sheets ---" << std::endl;

    for (auto& dino : dinos) {
        for (auto& size : sizes) {
            // Generate individual frame PNGs
            for (int p = 0; p < 4; p++) {
                sf::Image frame = dino.drawFunc(size.w, size.h, POSES[p], dino.color);
                std::string fn = outDir + "/" + dino.name + "_" + POSES[p].name + "_" + size.name + ".png";
                frame.saveToFile(fn);
                total++;
                std::cout << "  ✓ " << dino.name << "_" << POSES[p].name << "_" << size.name << ".png" << std::endl;
            }

            // Generate combined sprite sheet (4 frames side by side)
            int sheetW = size.w * 4;
            int sheetH = size.h;
            sf::Image sheet;
            sheet.create(sheetW, sheetH, sf::Color::Transparent);
            for (int p = 0; p < 4; p++) {
                sf::Image frame = dino.drawFunc(size.w, size.h, POSES[p], dino.color);
                sheet.copy(frame, p * size.w, 0, sf::IntRect(0, 0, size.w, size.h));
            }
            std::string sheetFn = outDir + "/" + dino.name + "_sheet_" + size.name + ".png";
            sheet.saveToFile(sheetFn);
            total++;
            std::cout << "  ✓ " << dino.name << "_sheet_" << size.name << ".png (sprite sheet)" << std::endl;
        }
    }

    // ============================================================
    // OBSTACLE TEXTURES
    // ============================================================
    std::cout << std::endl << "--- Generating Obstacle Textures ---" << std::endl;

    struct ObjDef {
        std::string name;
        sf::Image (*gen)(int, int);
        int w, h;
    };

    // Single-frame obstacles (rocks, coins, clouds, ground - no animation needed)
    ObjDef singleObstacles[] = {
        {"rock",         drawRock,          50, 40},
        {"coin",         drawCoin,          24, 24},
        {"cloud",        drawCloud,        120, 50},
        {"ground",       drawGroundTexture,128, 60},
    };

    for (auto& ob : singleObstacles) {
        sf::Image img = ob.gen(ob.w, ob.h);
        std::string fn = outDir + "/" + ob.name + ".png";
        img.saveToFile(fn);
        total++;
        std::cout << "  ✓ " << ob.name << ".png" << std::endl;

        sf::Image img2x = ob.gen(ob.w * 2, ob.h * 2);
        std::string fn2x = outDir + "/" + ob.name + "@2x.png";
        img2x.saveToFile(fn2x);
        total++;
        std::cout << "  ✓ " << ob.name << "@2x.png (retina)" << std::endl;
    }

    // Animated obstacles — generate sprite sheets
    struct AnimObjDef {
        std::string name;
        sf::Image (*sheetGen)(int, int);
        int fw, fh;
        int numFrames;
    };

    AnimObjDef animObstacles[] = {
        {"cactus_small", drawCactusSpriteSheet,         30, 50, 2},
        {"cactus_large", drawCactusSpriteSheet,         45, 70, 2},
        {"pterodactyl",  drawPterodactylSpriteSheet,    60, 40, 2},
    };

    for (auto& ob : animObstacles) {
        sf::Image sheet = ob.sheetGen(ob.fw, ob.fh);
        std::string fn = outDir + "/" + ob.name + ".png";
        sheet.saveToFile(fn);
        total++;
        std::cout << "  ✓ " << ob.name << ".png (sprite sheet, " << ob.numFrames << " frames)" << std::endl;

        // Also generate 2x version
        sf::Image sheet2x = ob.sheetGen(ob.fw * 2, ob.fh * 2);
        std::string fn2x = outDir + "/" + ob.name + "@2x.png";
        sheet2x.saveToFile(fn2x);
        total++;
        std::cout << "  ✓ " << ob.name << "@2x.png (retina sprite sheet)" << std::endl;
    }

    // ============================================================
    // POWER-UP TEXTURES
    // ============================================================
    std::cout << std::endl << "--- Generating Power-Up Textures ---" << std::endl;

    struct PowerUpDef {
        std::string name;
        sf::Color color;
        std::string symbol;
    };

    PowerUpDef powerups[] = {
        {"powerup_shield",  sf::Color(0, 200, 255),   "S"},
        {"powerup_magnet",  sf::Color(200, 0, 255),   "M"},
        {"powerup_double",  sf::Color(255, 200, 0),   "D"},
        {"powerup_speed",   sf::Color(0, 200, 50),    "»"},
        {"powerup_life",    sf::Color(255, 30, 30),   "♥"},
    };

    for (auto& pu : powerups) {
        sf::Image img = drawPowerUp(32, 32, pu.color, pu.symbol);
        std::string fn = outDir + "/" + pu.name + ".png";
        img.saveToFile(fn);
        total++;
        std::cout << "  ✓ " << pu.name << ".png" << std::endl;

        sf::Image img2x = drawPowerUp(64, 64, pu.color, pu.symbol);
        std::string fn2x = outDir + "/" + pu.name + "@2x.png";
        img2x.saveToFile(fn2x);
        total++;
        std::cout << "  ✓ " << pu.name << "@2x.png (retina)" << std::endl;
    }

    // ============================================================
    // SPECIAL TEXTURES
    // ============================================================
    std::cout << std::endl << "--- Generating Special Textures ---" << std::endl;

    // Lava
    {
        sf::RenderTexture rt;
        rt.create(64, 12);
        rt.clear(sf::Color::Transparent);
        sf::RectangleShape lava(sf::Vector2f(64, 12));
        lava.setFillColor(sf::Color(255, 80, 0));
        rt.draw(lava);
        // Orange glow spots
        sf::CircleShape spot(3);
        spot.setFillColor(sf::Color(255, 200, 50));
        spot.setPosition(5, 2);
        rt.draw(spot);
        spot.setPosition(25, 6);
        rt.draw(spot);
        spot.setPosition(45, 3);
        rt.draw(spot);
        rt.display();
        rt.getTexture().copyToImage().saveToFile(outDir + "/lava.png");
        total++;
        std::cout << "  ✓ lava.png" << std::endl;
    }

    // Stalactite
    {
        sf::RenderTexture rt;
        rt.create(20, 50);
        rt.clear(sf::Color::Transparent);
        sf::ConvexShape stal;
        stal.setPointCount(4);
        stal.setPoint(0, sf::Vector2f(2, 0));
        stal.setPoint(1, sf::Vector2f(18, 0));
        stal.setPoint(2, sf::Vector2f(16, 40));
        stal.setPoint(3, sf::Vector2f(4, 50));
        stal.setFillColor(sf::Color(130, 110, 90));
        stal.setOutlineColor(sf::Color(90, 75, 60));
        stal.setOutlineThickness(0.5f);
        rt.draw(stal);
        // Highlight
        sf::ConvexShape hl;
        hl.setPointCount(3);
        hl.setPoint(0, sf::Vector2f(4, 2));
        hl.setPoint(1, sf::Vector2f(8, 0));
        hl.setPoint(2, sf::Vector2f(6, 20));
        hl.setFillColor(sf::Color(160, 140, 120));
        rt.draw(hl);
        rt.display();
        rt.getTexture().copyToImage().saveToFile(outDir + "/stalactite.png");
        total++;
        std::cout << "  ✓ stalactite.png" << std::endl;
    }

    // Ground enemy
    {
        sf::Image groundEnemy = drawRock(40, 30);  // Reuse rock shape for ground enemy
        groundEnemy.saveToFile(outDir + "/ground_enemy.png");
        total++;
        std::cout << "  ✓ ground_enemy.png" << std::endl;
    }

    // Particle
    {
        sf::RenderTexture rt;
        rt.create(8, 8);
        rt.clear(sf::Color::Transparent);
        sf::CircleShape p(4);
        p.setFillColor(sf::Color::White);
        p.setPosition(0, 0);
        rt.draw(p);
        sf::CircleShape glow(3);
        glow.setFillColor(sf::Color(255, 255, 255, 150));
        glow.setPosition(1, 1);
        rt.draw(glow);
        rt.display();
        rt.getTexture().copyToImage().saveToFile(outDir + "/particle.png");
        total++;
        std::cout << "  ✓ particle.png" << std::endl;
    }

    // ============================================================
    // SUMMARY
    // ============================================================
    std::cout << std::endl;
    std::cout << "╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Generated " << total << " textures in " << outDir << "/     ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════╝" << std::endl;

    return 0;
}
