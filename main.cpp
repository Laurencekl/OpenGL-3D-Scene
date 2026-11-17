#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <random>
#include <vector>

#include "font.h"
#include "glb_model.h"

const GLfloat kWorldScale = 1.20f;

const GLfloat kPerspectiveFieldOfViewAngle = 45;
const GLfloat kPerspectiveNearZ = 0.1;
const GLfloat kPerspectiveFarZ = 180;
const GLfloat kFogStart = 30.0f;
const GLfloat kFogEnd = 60.0f;
const GLfloat kFogColor[] = {0.48f, 0.55f, 0.60f, 1.0f};
const GLfloat kSkyColor[] = {0.32f, 0.53f, 0.70f, 1.0f};
const GLfloat kSceneryBlockWidth = 44.0f;
const GLfloat kSceneryBlockLength = 46.0f;
GLuint scenery_display_list = 0;

const GLfloat kParkedCarPositionX = 3.4f;
const GLfloat kParkedCarPositionZ = -12.0f;

bool street_light_enabled = false;
bool f_key_was_pressed = false;

GlbModel model;

GlbModel street_lamp_model;
GlbModel traffic_cone_model;
GlbModel house_model;
GlbModel neymar_model;
GlbModel umbrella_model;
GlbModel beetle_model;
GlbModel cloud_model;
GlbModel pikachu_model;

GlbTexture road_texture;

const GLfloat kRoadHeight = -0.65f;
const GLfloat kLampScale = 0.85f;
const GLfloat kConeScale = 0.16f;
const GLfloat kHouseScale = 1.7f;
const GLfloat kCarRoadOffset = 0.40f;
const GLfloat kSidewalkHeight = kRoadHeight + 0.18f;
const GLfloat kGrassHeight = kRoadHeight + 0.14f;
GLfloat house_ground_y = 0;
GLfloat lamp_ground_y = 0;
GLfloat cone_ground_y = 0;
const GLfloat kNeymarScale = 0.42f;
const GLfloat kUmbrellaScale = 0.80f;
const GLfloat kBeetleScale = 0.85f;
const GLfloat kPikachuScale = 0.16f;
GLfloat neymar_ground_y = 0;
GLfloat beetle_ground_y = 0;
GLfloat pikachu_ground_y = 0;

const GLfloat kStreetLightHeight = 4.8f;

struct StreetLampPosition {
  GLfloat x;
  GLfloat z;
  GLfloat rotation_y;
};

const StreetLampPosition street_lamps[] = {
  {5.9f, -11.5f, 0},
  {-5.9f, -11.5f, 180},
  {5.9f, 1.0f, 0},
  {-5.9f, 1.0f, 180},
  {5.9f, -25.0f, 0},
  {-5.9f, -25.0f, 180}
};
const int kStreetLampCount = sizeof(street_lamps) / sizeof(street_lamps[0]);

struct BackgroundHousePosition {
  GLfloat x, z, rotation_y;
  GLint color;
};

const BackgroundHousePosition background_houses[] = {
  {-14, -59, 90, 0},
  {-14, -39, 90, 2},
  {-14, 1, 90, 3},
  {-14, 21, 90, 4},
  {-14, 41, 90, 5},
  {14, -59, -90, 3},
  {14, -39, -90, 4},
  {14, -19, -90, 0},
  {14, 1, -90, 1},
  {14, 21, -90, 5},
  {14, 41, -90, 2}
};

const aiColor4D kHouseColors[] = {
  {0.48f, 0.47f, 0.44f, 1},
  {0.36f, 0.39f, 0.41f, 1},
  {0.38f, 0.41f, 0.36f, 1},
  {0.55f, 0.49f, 0.39f, 1},
  {0.44f, 0.39f, 0.34f, 1},
  {0.58f, 0.56f, 0.51f, 1}
};
const GLint kHouseColorCount = sizeof(kHouseColors) / sizeof(kHouseColors[0]);
GLuint house_color_display_lists[kHouseColorCount] = {};

const GLfloat kHudHeight = 500;
const GLfloat kHudWidth = 500;
std::ostringstream hud_text;

double last_time = 0;
const int kRainDropCount = 1600;

struct MovingCloud {
  GLfloat x, y, z, scale, speed;
};
MovingCloud clouds[] = {
  {-14, 9.5f, -25, 2.2f, 0.65f},
  {4, 10.5f, -32, 2.6f, 0.50f},
  {-8, 14, -8, 2.1f, 0.75f},
  {14, 12, -15, 2.4f, 0.60f},
  {-20, 13, -38, 2.4f, 0.55f}
};
const int kCloudCount = sizeof(clouds) / sizeof(clouds[0]);
GLfloat cloud_bottom_y = -1.35f;

struct RainDrop {
  GLfloat offset_x, y, offset_z, speed, length;
  int cloud_index;
};
std::vector<RainDrop> rain_drops;
std::mt19937 weather_random(13);

GLfloat random_weather_value(GLfloat minimum, GLfloat maximum) {
  return std::uniform_real_distribution<GLfloat>(minimum, maximum)(weather_random);
}

void reset_rain_drop(RainDrop& drop, bool distribute_height = false) {
  const MovingCloud& cloud = clouds[drop.cloud_index];
  const GLfloat angle = random_weather_value(0, 6.2831853f);
  const GLfloat radius = std::sqrt(random_weather_value(0, 1));
  drop.offset_x = std::cos(angle) * radius * cloud.scale * 1.25f;
  drop.offset_z = std::sin(angle) * radius * cloud.scale * 2.10f;
  const GLfloat top = fmaxf(kRoadHeight + 0.1f,
      cloud.y + cloud_bottom_y * cloud.scale - drop.length - 0.05f);
  drop.y = distribute_height ? random_weather_value(kRoadHeight, top) : top;
}

void initialize_weather() {
  rain_drops.resize(kRainDropCount);
  for (int i = 0; i < kRainDropCount; ++i) {
    RainDrop& drop = rain_drops[i];
    drop.cloud_index = i % kCloudCount;
    drop.speed = random_weather_value(9, 14);
    drop.length = random_weather_value(0.22f, 0.40f);
    reset_rain_drop(drop, true);
  }
}


const GLfloat kCameraSpeed = 9.0f;
const GLfloat kMouseSensitivity = 0.1f;
const GLfloat kCameraEyeHeight = 3.0f;
const GLfloat kCameraRadius = 0.25f;
const GLfloat kCameraPitchLimit = 85.0f;
const GLfloat kCameraMinimumX = -22.0f * kWorldScale + kCameraRadius;
const GLfloat kCameraMaximumX = 22.0f * kWorldScale - kCameraRadius;
const GLfloat kCameraMinimumZ = -38.0f * kWorldScale + kCameraRadius;
const GLfloat kCameraMaximumZ = 8.0f * kWorldScale - kCameraRadius;
const GLfloat kDefaultCameraEyePositionX = -2.0f;
const GLfloat kDefaultCameraEyePositionZ = -22.9f;
const GLfloat kDefaultCameraYaw = 152.5f;
const GLfloat kDefaultCameraPitch = 0.0f;
const GLfloat kCameraUpX = 0, kCameraUpY = 1, kCameraUpZ = 0;
GLfloat camera_eye_x = kDefaultCameraEyePositionX;
GLfloat camera_eye_y = kRoadHeight * kWorldScale + kCameraEyeHeight;
GLfloat camera_eye_z = kDefaultCameraEyePositionZ;
GLfloat camera_direction_x = 0, camera_direction_y = 0, camera_direction_z = -1;
GLfloat camera_yaw = kDefaultCameraYaw, camera_pitch = kDefaultCameraPitch;
double camera_last_time = 0;

GLfloat camera_ground_height(GLfloat world_x, GLfloat world_z) {
  const GLfloat x = world_x / kWorldScale, z = world_z / kWorldScale;
  const bool sidewalk = (x >= -7.5f && x <= -5) || (x >= 5 && x <= 7.5f);
  const bool house_path = x >= -14 && x <= -7.5f && z >= -18 && z <= -16.4f;
  const GLfloat ground = (sidewalk || house_path) ? kSidewalkHeight :
                         (x > -5 && x < 5 ? kRoadHeight : kGrassHeight);
  return ground * kWorldScale;
}

void update_camera_direction() {
  const GLfloat yaw = camera_yaw * 3.14159265f / 180;
  const GLfloat pitch = camera_pitch * 3.14159265f / 180;
  camera_direction_x = std::cos(yaw) * std::cos(pitch);
  camera_direction_y = std::sin(pitch);
  camera_direction_z = std::sin(yaw) * std::cos(pitch);
}

static void mouse_read(GLFWwindow*, double mouse_x, double mouse_y) {
  static bool initialized = false;
  static double last_x = 0, last_y = 0;
  if (initialized) {
    camera_yaw += static_cast<GLfloat>(mouse_x - last_x) * kMouseSensitivity;
    camera_pitch += static_cast<GLfloat>(last_y - mouse_y) * kMouseSensitivity;
    camera_pitch = std::clamp(camera_pitch, -kCameraPitchLimit, kCameraPitchLimit);
    update_camera_direction();
  }
  initialized = true;
  last_x = mouse_x; last_y = mouse_y;
}

void reset_camera() {
  camera_eye_x = kDefaultCameraEyePositionX;
  camera_eye_z = kDefaultCameraEyePositionZ;
  camera_eye_y = camera_ground_height(camera_eye_x, camera_eye_z) + kCameraEyeHeight;
  camera_yaw = kDefaultCameraYaw;
  camera_pitch = kDefaultCameraPitch;
  update_camera_direction();
}

bool camera_position_is_free(GLfloat x, GLfloat z);

void update_camera(GLFWwindow* window) {
  const double now = glfwGetTime();
  const GLfloat dt = static_cast<GLfloat>(std::clamp(now - camera_last_time, 0.0, 0.1));
  camera_last_time = now;
  const GLfloat forward = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? 1.0f : 0.0f)
                       - (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? 1.0f : 0.0f);
  const GLfloat right = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? 1.0f : 0.0f)
                     - (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? 1.0f : 0.0f);
  const GLfloat yaw = camera_yaw * 3.14159265f / 180;
  GLfloat dx = std::cos(yaw) * forward - std::sin(yaw) * right;
  GLfloat dz = std::sin(yaw) * forward + std::cos(yaw) * right;
  const GLfloat length = std::sqrt(dx * dx + dz * dz);
  if (length > 0) {
    dx *= kCameraSpeed * dt / length; dz *= kCameraSpeed * dt / length;
  }
  const int steps = std::max(1, static_cast<int>(std::ceil(
      std::sqrt(dx * dx + dz * dz) / (kCameraRadius * 0.5f))));
  for (int i = 0; i < steps; ++i) {
    const GLfloat x = std::clamp(camera_eye_x + dx / steps, kCameraMinimumX, kCameraMaximumX);
    if (camera_position_is_free(x, camera_eye_z)) camera_eye_x = x;
    const GLfloat z = std::clamp(camera_eye_z + dz / steps, kCameraMinimumZ, kCameraMaximumZ);
    if (camera_position_is_free(camera_eye_x, z)) camera_eye_z = z;
  }
  camera_eye_y = camera_ground_height(camera_eye_x, camera_eye_z) + kCameraEyeHeight;
}

struct CollisionBox {
  GLfloat min_x, min_y, min_z, max_x, max_y, max_z;
};
std::vector<CollisionBox> scene_collisions;

CollisionBox model_collision_bounds(const GlbModel& object) {
  const GLfloat limit = std::numeric_limits<GLfloat>::max();
  CollisionBox box{limit, limit, limit, -limit, -limit, -limit};
  for (unsigned int i = 0; i < object.scene->mNumMeshes; ++i) {
    const aiMesh* mesh = object.scene->mMeshes[i];
    for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
      const aiVector3D& v = mesh->mVertices[j];
      const GLfloat x = (v.x - object.center_x) * object.normalization_scale;
      const GLfloat y = (v.y - object.center_y) * object.normalization_scale;
      const GLfloat z = (v.z - object.center_z) * object.normalization_scale;
      box.min_x = fminf(box.min_x, x); box.max_x = fmaxf(box.max_x, x);
      box.min_y = fminf(box.min_y, y); box.max_y = fmaxf(box.max_y, y);
      box.min_z = fminf(box.min_z, z); box.max_z = fmaxf(box.max_z, z);
    }
  }
  return box;
}

CollisionBox transform_collision_box(const CollisionBox& local,
    GLfloat x, GLfloat y, GLfloat z, GLfloat sx, GLfloat sy, GLfloat sz,
    GLfloat rx = 0, GLfloat ry = 0, GLfloat rz = 0, GLfloat offset_y = 0) {
  const GLfloat limit = std::numeric_limits<GLfloat>::max();
  CollisionBox box{limit, limit, limit, -limit, -limit, -limit};
  const GLfloat ax = rx * 3.14159265f / 180, ay = ry * 3.14159265f / 180;
  const GLfloat az = rz * 3.14159265f / 180;
  for (int corner = 0; corner < 8; ++corner) {
    GLfloat px = ((corner & 1) ? local.max_x : local.min_x) * sx;
    GLfloat py = (((corner & 2) ? local.max_y : local.min_y) + offset_y) * sy;
    GLfloat pz = ((corner & 4) ? local.max_z : local.min_z) * sz;
    GLfloat a = px * std::cos(az) - py * std::sin(az);
    py = px * std::sin(az) + py * std::cos(az); px = a;
    a = px * std::cos(ay) + pz * std::sin(ay);
    pz = -px * std::sin(ay) + pz * std::cos(ay); px = a;
    a = py * std::cos(ax) - pz * std::sin(ax);
    pz = py * std::sin(ax) + pz * std::cos(ax); py = a;
    px = (px + x) * kWorldScale;
    py = (py + y) * kWorldScale;
    pz = (pz + z) * kWorldScale;
    box.min_x = fminf(box.min_x, px); box.max_x = fmaxf(box.max_x, px);
    box.min_y = fminf(box.min_y, py); box.max_y = fmaxf(box.max_y, py);
    box.min_z = fminf(box.min_z, pz); box.max_z = fmaxf(box.max_z, pz);
  }
  return box;
}

void initialize_collisions() {
  scene_collisions.clear();
  auto add = [](const GlbModel& object, GLfloat x, GLfloat y, GLfloat z,
                GLfloat scale, GLfloat ry = 0) {
    scene_collisions.push_back(transform_collision_box(model_collision_bounds(object),
        x, y, z, scale, scale, scale, 0, ry));
  };
  add(house_model, -14, house_ground_y, -19, kHouseScale, 90);
  for (const BackgroundHousePosition& house : background_houses) {
    add(house_model, house.x, house_ground_y, house.z, kHouseScale, house.rotation_y);
  }
  add(beetle_model, -13, beetle_ground_y, -12.3f, kBeetleScale, -90);
  add(neymar_model, -7.9f, neymar_ground_y, -17.2f, kNeymarScale, 90);
  add(pikachu_model, -8.1f, pikachu_ground_y, -15.8f, kPikachuScale, 90);
  add(traffic_cone_model, 4.2f, cone_ground_y, -4.5f, kConeScale);
  CollisionBox pole = model_collision_bounds(street_lamp_model);
  const GLfloat base_top = pole.min_y + (pole.max_y - pole.min_y) * 0.08f;
  pole.min_x = pole.min_z = std::numeric_limits<GLfloat>::max();
  pole.max_x = pole.max_z = std::numeric_limits<GLfloat>::lowest();
  for (unsigned int i = 0; i < street_lamp_model.scene->mNumMeshes; ++i) {
    const aiMesh* mesh = street_lamp_model.scene->mMeshes[i];
    for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
      const aiVector3D& v = mesh->mVertices[j];
      if ((v.y - street_lamp_model.center_y) * street_lamp_model.normalization_scale > base_top) continue;
      const GLfloat x = (v.x - street_lamp_model.center_x) * street_lamp_model.normalization_scale;
      const GLfloat z = (v.z - street_lamp_model.center_z) * street_lamp_model.normalization_scale;
      pole.min_x = fminf(pole.min_x, x); pole.max_x = fmaxf(pole.max_x, x);
      pole.min_z = fminf(pole.min_z, z); pole.max_z = fmaxf(pole.max_z, z);
    }
  }
  for (const StreetLampPosition& lamp : street_lamps) {
    scene_collisions.push_back(transform_collision_box(pole,
        lamp.x, lamp_ground_y, lamp.z, kLampScale, kLampScale, kLampScale, 0, lamp.rotation_y));
  }
  add(model, kParkedCarPositionX, kCarRoadOffset, kParkedCarPositionZ, 1);
}

bool camera_hits_box(GLfloat x, GLfloat z, const CollisionBox& box) {
  const GLfloat feet = camera_ground_height(x, z);
  if (box.max_y <= feet + 0.02f || box.min_y >= feet + kCameraEyeHeight) return false;
  const GLfloat nearest_x = std::clamp(x, box.min_x, box.max_x);
  const GLfloat nearest_z = std::clamp(z, box.min_z, box.max_z);
  const GLfloat dx = x - nearest_x, dz = z - nearest_z;
  return dx * dx + dz * dz < kCameraRadius * kCameraRadius;
}

bool camera_position_is_free(GLfloat x, GLfloat z) {
  for (const CollisionBox& box : scene_collisions) {
    if (camera_hits_box(x, z, box)) return false;
  }
  return true;
}


void keyboard_read(GLFWwindow* window) {
  hud_text.str("");

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  const bool f_key_is_pressed = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
  if (f_key_is_pressed && !f_key_was_pressed) {
    street_light_enabled = !street_light_enabled;
    hud_text << (street_light_enabled ? "Poste ligado " : "Poste desligado ");
  }
  f_key_was_pressed = f_key_is_pressed;

  hud_text << "F: postes " << (street_light_enabled ? "ON " : "OFF ");

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    reset_camera();

    hud_text << "Redefinido! ";
  }

}

void animate() {
  const double current_time = glfwGetTime();
  const double delta_time = current_time - last_time;
  last_time = current_time;

  for (MovingCloud& cloud : clouds) {
    cloud.x += cloud.speed * delta_time;
    if (cloud.x > 30) cloud.x = -30 + std::fmod(cloud.x - 30, 60.0f);
  }
  for (RainDrop& drop : rain_drops) {
    drop.y -= drop.speed * delta_time;
    if (drop.y < kRoadHeight) {
      reset_rain_drop(drop);
    }
  }
}

void draw_rain() {
  glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT |
               GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);
  glLineWidth(1);
  glColor4f(0.66f, 0.78f, 0.90f, 0.65f);
  glBegin(GL_LINES);
  for (const RainDrop& drop : rain_drops) {
    const MovingCloud& cloud = clouds[drop.cloud_index];
    const GLfloat x = cloud.x + drop.offset_x;
    const GLfloat z = cloud.z + drop.offset_z;
    glVertex3f(x, drop.y, z);
    glVertex3f(x + 0.04f, drop.y + drop.length, z);
  }
  glEnd();
  glPopAttrib();
}

void resize_window(GLFWwindow* window) {
  int window_width, window_height;
  glfwGetFramebufferSize(window, &window_width, &window_height);
  glViewport(0, 0, window_width, window_height);
}

void draw_ground_tile(GLfloat x1, GLfloat z1, GLfloat x2, GLfloat z2, GLfloat y,
                      GLfloat texture_scale = 1) {
  glNormal3f(0, 1, 0);
  glTexCoord2f(x1 * texture_scale, z1 * texture_scale); glVertex3f(x1, y, z1);
  glTexCoord2f(x1 * texture_scale, z2 * texture_scale); glVertex3f(x1, y, z2);
  glTexCoord2f(x2 * texture_scale, z2 * texture_scale); glVertex3f(x2, y, z2);
  glTexCoord2f(x2 * texture_scale, z1 * texture_scale); glVertex3f(x2, y, z1);
}

void draw_sidewalk(GLfloat left, GLfloat right) {
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  for (int z = -38; z < 8; ++z) {
    glColor3f(0.48f, 0.49f, 0.50f);
    draw_ground_tile(left, z, right, z + 1, kSidewalkHeight);
    glColor3f(0.68f, 0.69f, 0.70f);
    draw_ground_tile(left + 0.025f, z + 0.025f, right - 0.025f, z + 0.975f,
                     kSidewalkHeight + 0.002f);
  }
  glColor3f(0.56f, 0.57f, 0.58f);
  glNormal3f(-1, 0, 0);
  glVertex3f(left, kRoadHeight, -38); glVertex3f(left, kRoadHeight, 8);
  glVertex3f(left, kSidewalkHeight, 8); glVertex3f(left, kSidewalkHeight, -38);
  glNormal3f(1, 0, 0);
  glVertex3f(right, kRoadHeight, 8); glVertex3f(right, kRoadHeight, -38);
  glVertex3f(right, kSidewalkHeight, -38); glVertex3f(right, kSidewalkHeight, 8);
  glEnd();
}

void draw_grass(GLfloat left, GLfloat right) {
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  for (GLfloat x = left; x < right; x += 0.5f) {
    for (GLfloat z = -38; z < 8; z += 0.5f) {
      const int tile = static_cast<int>((x + 24) * 2) * 73 + static_cast<int>((z + 38) * 2) * 37;
      const GLfloat shade = (tile % 11) * 0.003f;
      glColor3f(0.20f + shade, 0.43f + shade, 0.12f + shade);
      draw_ground_tile(x, z, fminf(x + 0.5f, right), z + 0.5f, kGrassHeight);
    }
  }
  glEnd();
}

void draw_road(bool draw_house_path = true) {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, road_texture.id);
  glColor3ub(255, 255, 255);

  glBegin(GL_QUADS);
  for (GLfloat x = -5; x < 5; x += 0.5f) {
    for (GLfloat z = -38; z < 8; z += 0.5f) {
      draw_ground_tile(x, z, x + 0.5f, z + 0.5f, kRoadHeight, 0.22f);
    }
  }
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glColor3f(0.95f, 0.78f, 0.18f);
  for (int z = -37; z < 7; z += 4) {
    draw_ground_tile(-0.06f, z, 0.06f, z + 2, kRoadHeight + 0.005f);
  }
  glColor3f(0.9f, 0.9f, 0.86f);
  for (int z = -38; z < 8; ++z) {
    draw_ground_tile(-4.7f, z, -4.6f, z + 1, kRoadHeight + 0.005f);
    draw_ground_tile(4.6f, z, 4.7f, z + 1, kRoadHeight + 0.005f);
  }
  glEnd();
  draw_sidewalk(-7.5f, -5.0f);
  draw_sidewalk(5.0f, 7.5f);
  draw_grass(-22.0f, -7.5f);
  draw_grass(7.5f, 22.0f);
  if (!draw_house_path) return;
  glColor3f(0.67f, 0.65f, 0.60f);
  glBegin(GL_QUADS);
  for (GLfloat x = -14; x < -7.5f; x += 0.5f) {
    draw_ground_tile(x, -18.0f, x + 0.5f, -16.4f, kSidewalkHeight + 0.004f);
  }
  glEnd();
}

bool initialize_scenery_blocks() {
  scenery_display_list = glGenLists(1);
  if (!scenery_display_list) return false;
  glNewList(scenery_display_list, GL_COMPILE);
  for (GLint x = -1; x <= 1; ++x) {
    for (GLint z = -1; z <= 1; ++z) {
      if (x == 0 && z == 0) continue;
      glPushMatrix();
      glTranslatef(x * kSceneryBlockWidth, 0, z * kSceneryBlockLength);
      draw_road(false);
      glPopMatrix();
    }
  }
  glEndList();
  return true;
}

GLfloat model_ground_position(const GlbModel& scene_model, GLfloat scale, GLfloat ground) {
  GLfloat minimum_y = std::numeric_limits<GLfloat>::max();
  for (unsigned int i = 0; i < scene_model.scene->mNumMeshes; ++i) {
    const aiMesh* mesh = scene_model.scene->mMeshes[i];
    for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
      minimum_y = fminf(minimum_y, mesh->mVertices[j].y);
    }
  }
  return ground - (minimum_y - scene_model.center_y) * scene_model.normalization_scale * scale;
}

void draw_scene_object(const GlbModel& scene_model, GLfloat x, GLfloat y, GLfloat z,
                       GLfloat scale, GLfloat rotation_x = 0, GLfloat rotation_y = 0) {
  glPushMatrix();
  glTranslatef(x, y, z);
  glRotatef(rotation_x, 1, 0, 0);
  glRotatef(rotation_y, 0, 1, 0);
  glScalef(scale, scale, scale);

  draw_model_display_list(scene_model);
  glPopMatrix();
}

bool initialize_house_colors() {
  unsigned int wall_material = house_model.scene->mNumMaterials;
  for (unsigned int i = 0; i < house_model.scene->mNumMaterials; ++i) {
    aiString name;
    house_model.scene->mMaterials[i]->Get(AI_MATKEY_NAME, name);
    if (std::string(name.C_Str()) == "Material.013") wall_material = i;
  }
  const auto texture = house_model.textures.find(wall_material);
  if (texture == house_model.textures.end()) return false;
  const GlbTexture original_texture = texture->second;
  const aiColor4D original_color = house_model.material_colors.at(wall_material);
  house_model.textures.erase(wall_material);
  bool success = true;
  for (GLint i = 0; i < kHouseColorCount; ++i) {
    house_color_display_lists[i] = glGenLists(1);
    if (!house_color_display_lists[i]) { success = false; break; }
    house_model.material_colors[wall_material] = kHouseColors[i];
    glNewList(house_color_display_lists[i], GL_COMPILE);
    draw_model(house_model);
    glEndList();
  }
  house_model.textures[wall_material] = original_texture;
  house_model.material_colors[wall_material] = original_color;
  return success;
}

void draw_colored_house(const BackgroundHousePosition& house) {
  glPushMatrix();
  glTranslatef(house.x, house_ground_y, house.z);
  glRotatef(house.rotation_y, 0, 1, 0);
  glScalef(kHouseScale, kHouseScale, kHouseScale);
  glCallList(house_color_display_lists[house.color]);
  glPopMatrix();
}

void draw_neymar_with_umbrella() {
  glPushMatrix();
  glTranslatef(-7.9f, neymar_ground_y, -17.2f);
  glRotatef(90, 0, 1, 0);
  glScalef(kNeymarScale, kNeymarScale, kNeymarScale);
  draw_model_display_list(neymar_model);

  glTranslatef(-0.597523f, 1.05617f, 0.633052f);
  glScalef(kUmbrellaScale, kUmbrellaScale, kUmbrellaScale);
  glTranslatef(umbrella_model.center_x * umbrella_model.normalization_scale,
               (umbrella_model.center_y - 0.10f) * umbrella_model.normalization_scale,
               umbrella_model.center_z * umbrella_model.normalization_scale);
  draw_model_display_list(umbrella_model);
  glPopMatrix();
}

void draw_sky() {
  glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_FOG);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glShadeModel(GL_SMOOTH);
  glPushMatrix();
  glTranslatef(camera_eye_x, camera_eye_y, camera_eye_z);
  const GLfloat radius = kPerspectiveFarZ * 0.75f;
  const GLfloat radians = 3.14159265f / 180;
  for (GLint latitude = -90; latitude < 90; latitude += 10) {
    glBegin(GL_QUAD_STRIP);
    for (GLint longitude = 0; longitude <= 360; longitude += 15) {
      for (GLint edge = 0; edge < 2; ++edge) {
        const GLfloat angle = (latitude + edge * 10) * radians;
        const GLfloat height = std::sin(angle);
        const GLfloat blue = std::clamp(height * 3, 0.0f, 1.0f);
        glColor3f(kFogColor[0] * (1 - blue) + kSkyColor[0] * blue,
                  kFogColor[1] * (1 - blue) + kSkyColor[1] * blue,
                  kFogColor[2] * (1 - blue) + kSkyColor[2] * blue);
        glVertex3f(radius * std::cos(angle) * std::cos(longitude * radians),
                   radius * height,
                   radius * std::cos(angle) * std::sin(longitude * radians));
      }
    }
    glEnd();
  }
  glPopMatrix();
  glPopAttrib();
}

void draw(GLFWwindow* window) {
  int window_width, window_height;
  glfwGetFramebufferSize(window, &window_width, &window_height);
  GLdouble aspect_ratio = (GLdouble)window_width / window_height;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  GLdouble field_of_view = kPerspectiveFieldOfViewAngle;
  GLdouble near = kPerspectiveNearZ;
  GLdouble far = kPerspectiveFarZ;
  gluPerspective(field_of_view, aspect_ratio, near, far);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  GLfloat camera_center_x = (camera_eye_x + camera_direction_x);
  GLfloat camera_center_y = (camera_eye_y + camera_direction_y);
  GLfloat camera_center_z = (camera_eye_z + camera_direction_z);
  gluLookAt(camera_eye_x, camera_eye_y, camera_eye_z, camera_center_x, camera_center_y,
            camera_center_z, kCameraUpX, kCameraUpY, kCameraUpZ);

  draw_sky();

  const GLfloat scene_ambient[] = {0.45f, 0.45f, 0.45f, 1};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, scene_ambient);
  const GLfloat sky_direction[] = {-0.4f, 1, 0.3f, 0};
  glLightfv(GL_LIGHT7, GL_POSITION, sky_direction);

  glPushMatrix();
  glScalef(kWorldScale, kWorldScale, kWorldScale);

  for (int i = 0; i < kStreetLampCount; ++i) {
    const GLenum light = GL_LIGHT0 + i;
    if (street_light_enabled) {
      glEnable(light);
      GLfloat position[] = {street_lamps[i].x, kStreetLightHeight,
                            street_lamps[i].z, 1.0f};
      glLightfv(light, GL_POSITION, position);
    } else {
      glDisable(light);
    }
  }

  glPushAttrib(GL_LIGHTING_BIT);
  const GLfloat cloud_emission[] = {0.18f, 0.20f, 0.22f, 1};
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, cloud_emission);
  for (const MovingCloud& cloud : clouds) {
    draw_scene_object(cloud_model, cloud.x, cloud.y, cloud.z, cloud.scale);
  }
  glPopAttrib();
  glCallList(scenery_display_list);
  draw_road();
  for (const BackgroundHousePosition& house : background_houses) {
    draw_colored_house(house);
  }
  for (const StreetLampPosition& lamp : street_lamps) {
    draw_scene_object(street_lamp_model, lamp.x, lamp_ground_y, lamp.z,
                      kLampScale, 0, lamp.rotation_y);
  }
  draw_scene_object(traffic_cone_model, 4.2f, cone_ground_y, -4.5f, kConeScale);
  draw_scene_object(house_model, -14.0f, house_ground_y, -19.0f, kHouseScale, 0, 90);
  draw_scene_object(beetle_model, -13.0f, beetle_ground_y, -12.3f, kBeetleScale, 0, -90);

  draw_neymar_with_umbrella();
  draw_scene_object(pikachu_model, -8.1f, pikachu_ground_y, -15.8f, kPikachuScale, 0, 90);

  glPushMatrix();
  glTranslatef(kParkedCarPositionX, kCarRoadOffset, kParkedCarPositionZ);

  glEnable(GL_TEXTURE_2D);
  draw_model_display_list(model);
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();
  draw_rain();
  glPopMatrix();
}

void draw_hud(GLFWwindow* window) {
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_FOG);
  int window_width, window_height;
  glfwGetFramebufferSize(window, &window_width, &window_height);
  GLdouble aspect_ratio = (GLdouble)window_width / window_height;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  GLdouble left = 0;
  GLdouble right = kHudWidth;
  GLdouble bottom = 0;
  GLdouble top = kHudHeight;
  if (window_width > window_height) {
    gluOrtho2D((left * aspect_ratio), (right * aspect_ratio), bottom, top);
  } else {
    gluOrtho2D(left, right, (bottom / aspect_ratio), (top / aspect_ratio));
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  {
    glColor3ub(255, 255, 255);
    draw_text(5, 5, hud_text.str());

  }
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_LIGHTING);
  glPopAttrib();
}

int main() {
  if (!glfwInit()) {
    std::cerr << "Falha ao inicializar GLFW" << std::endl;
    return EXIT_FAILURE;
  }

  GLFWwindow* window = glfwCreateWindow(800, 800, "", NULL, NULL);
  if (!window) {
    std::cerr << "Falha ao criar a janela GLFW" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwSetWindowPos(window, 0, 0);
  glfwSetWindowTitle(window, "Carregando modelo 3D... aguarde");
  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Falha ao inicializar GLEW" << std::endl;
    return EXIT_FAILURE;
  }

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

  glEnable(GL_NORMALIZE);

  GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat light_diffuse[] = {1.15f, 1.00f, 0.78f, 1.0f};
  for (int i = 0; i < kStreetLampCount; ++i) {
    const GLenum light = GL_LIGHT0 + i;
    glLightfv(light, GL_AMBIENT, light_ambient);
    glLightfv(light, GL_DIFFUSE, light_diffuse);
    glLightf(light, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(light, GL_LINEAR_ATTENUATION, 0.10f / kWorldScale);
    glLightf(light, GL_QUADRATIC_ATTENUATION, 0.045f / (kWorldScale * kWorldScale));
  }
  const GLfloat sky_diffuse[] = {0.40f, 0.43f, 0.48f, 1};
  glLightfv(GL_LIGHT7, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT7, GL_DIFFUSE, sky_diffuse);
  glEnable(GL_LIGHT7);
  glClearColor(kFogColor[0], kFogColor[1], kFogColor[2], kFogColor[3]);
  glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogfv(GL_FOG_COLOR, kFogColor);
  glFogf(GL_FOG_START, kFogStart);
  glFogf(GL_FOG_END, kFogEnd);
  glHint(GL_FOG_HINT, GL_NICEST);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  resize_window(window);
  hud_text.str("");
  hud_text << "CARREGANDO MODELO 3D...";
  draw_hud(window);
  glfwSwapBuffers(window);
  glfwPollEvents();
  std::cout << "Carregando o modelo 3D, aguarde..." << std::endl;

  if (!load_model(model, "models/sports_car.glb")) {
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  if (!load_texture_external(road_texture, "textures/aerial_asphalt_01_diff_1k.jpg") ||
      !load_model(street_lamp_model, "models/street_lamp/Street_Light_A_02.obj") ||
      !load_model(traffic_cone_model, "models/traffic_cone.glb") ||
      !load_model(house_model, "models/house.glb") ||
      !load_model(neymar_model, "models/neymar_holding.glb") ||
      !load_model(umbrella_model, "models/umbrella/Umbrella.obj") ||
      !load_model(beetle_model, "models/beetle/VWbeetle.obj", false) ||
      !load_model(cloud_model, "models/cloud/cloud.obj", true, true) ||
      !load_model(pikachu_model, "models/pikachu/pikachu.obj", true, true)) {
    destroy_model(model);
    destroy_model(street_lamp_model);
    destroy_model(traffic_cone_model);
    destroy_model(house_model);
    destroy_model(neymar_model);
    destroy_model(umbrella_model);
    destroy_model(beetle_model);
    destroy_model(cloud_model);
    destroy_model(pikachu_model);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  house_ground_y = model_ground_position(house_model, kHouseScale, kGrassHeight);
  lamp_ground_y = model_ground_position(street_lamp_model, kLampScale, kSidewalkHeight);
  cone_ground_y = model_ground_position(traffic_cone_model, kConeScale, kRoadHeight);
  neymar_ground_y = model_ground_position(neymar_model, kNeymarScale, kSidewalkHeight + 0.004f);
  beetle_ground_y = model_ground_position(beetle_model, kBeetleScale, kGrassHeight);
  pikachu_ground_y = model_ground_position(pikachu_model, kPikachuScale, kGrassHeight);
  cloud_bottom_y = -model_ground_position(cloud_model, 1, 0);
  for (auto& material : cloud_model.material_colors) {
    material.second = aiColor4D(0.78f, 0.80f, 0.83f, 1);
  }

  if (!create_model_display_list(model) || !create_model_display_list(street_lamp_model) ||
      !create_model_display_list(traffic_cone_model) || !create_model_display_list(house_model) ||
      !create_model_display_list(neymar_model) || !create_model_display_list(umbrella_model) ||
      !create_model_display_list(beetle_model) || !create_model_display_list(cloud_model) ||
      !create_model_display_list(pikachu_model)) {
    destroy_model(model);
    destroy_model(street_lamp_model);
    destroy_model(traffic_cone_model);
    destroy_model(house_model);
    destroy_model(neymar_model);
    destroy_model(umbrella_model);
    destroy_model(beetle_model);
    destroy_model(cloud_model);
    destroy_model(pikachu_model);
    if (road_texture.id) glDeleteTextures(1, &road_texture.id);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  if (!initialize_house_colors() || !initialize_scenery_blocks()) {
    std::cerr << "Falha ao preparar casas e blocos do cenario" << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  initialize_weather();
  initialize_collisions();
  reset_camera();
  last_time = glfwGetTime();
  camera_last_time = last_time;
  glfwSetWindowTitle(window, "Aula 15 - Camera e colisoes");
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_read);

  while (!glfwWindowShouldClose(window)) {
    keyboard_read(window);
    update_camera(window);
    resize_window(window);
    animate();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw(window);
    draw_hud(window);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteLists(scenery_display_list, 1);
  for (GLuint list : house_color_display_lists) {
    if (list) glDeleteLists(list, 1);
  }
  destroy_model(model);
  destroy_model(street_lamp_model);
  destroy_model(traffic_cone_model);
  destroy_model(house_model);
  destroy_model(neymar_model);
  destroy_model(umbrella_model);
  destroy_model(beetle_model);
  destroy_model(cloud_model);
  destroy_model(pikachu_model);
  if (road_texture.id) glDeleteTextures(1, &road_texture.id);

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
