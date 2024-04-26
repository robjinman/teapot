#include <iostream>
#include <array>
#include <fstream>
#include <vector>
#include <sstream>
#include <chrono>
#include <thread>

const double PI = 3.14159265359;

struct Vec4f {
  double x;
  double y;
  double z;
  double w;
};

struct Vec2f {
  double x;
  double y;
};

struct Vec2i {
  int x;
  int y;
};

struct Vec3f {
  double x;
  double y;
  double z;
};

class Mat4 {
  public:
    Mat4() : m_data{
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    } {}

    Mat4(const std::array<double, 16>& data) {
      for (uint32_t i = 0; i < 16; ++i) {
        const uint32_t r = i / 4;
        const uint32_t c = i % 4;
        set(c, r, data[i]);
      }
    }

    double at(uint32_t col, uint32_t row) const {
      return m_data[col * 4 + row];
    }

    void set(uint32_t col, uint32_t row, double value) {
      m_data[col * 4 + row] = value;
    }

    Vec4f operator*(const Vec4f& v) const {
      return Vec4f{
        at(0, 0) * v.x + at(1, 0) * v.y + at(2, 0)  * v.z + at(3, 0) * v.w,
        at(0, 1) * v.x + at(1, 1) * v.y + at(2, 1)  * v.z + at(3, 1) * v.w,
        at(0, 2) * v.x + at(1, 2) * v.y + at(2, 2)  * v.z + at(3, 2) * v.w,
        at(0, 3) * v.x + at(1, 3) * v.y + at(2, 3)  * v.z + at(3, 3) * v.w
      };
    }

    Mat4 operator*(const Mat4& m) const {
      return Mat4{{
        at(0, 0) * m.at(0, 0) + at(1, 0) * m.at(0, 1) + at(2, 0)  * m.at(0, 2) + at(3, 0) * m.at(0, 3),
        at(0, 0) * m.at(1, 0) + at(1, 0) * m.at(1, 1) + at(2, 0)  * m.at(1, 2) + at(3, 0) * m.at(1, 3),
        at(0, 0) * m.at(2, 0) + at(1, 0) * m.at(2, 1) + at(2, 0)  * m.at(2, 2) + at(3, 0) * m.at(2, 3),
        at(0, 0) * m.at(3, 0) + at(1, 0) * m.at(3, 1) + at(2, 0)  * m.at(3, 2) + at(3, 0) * m.at(3, 3),

        at(0, 1) * m.at(0, 0) + at(1, 1) * m.at(0, 1) + at(2, 1)  * m.at(0, 2) + at(3, 1) * m.at(0, 3),
        at(0, 1) * m.at(1, 0) + at(1, 1) * m.at(1, 1) + at(2, 1)  * m.at(1, 2) + at(3, 1) * m.at(1, 3),
        at(0, 1) * m.at(2, 0) + at(1, 1) * m.at(2, 1) + at(2, 1)  * m.at(2, 2) + at(3, 1) * m.at(2, 3),
        at(0, 1) * m.at(3, 0) + at(1, 1) * m.at(3, 1) + at(2, 1)  * m.at(3, 2) + at(3, 1) * m.at(3, 3),

        at(0, 2) * m.at(0, 0) + at(1, 2) * m.at(0, 1) + at(2, 2)  * m.at(0, 2) + at(3, 2) * m.at(0, 3),
        at(0, 2) * m.at(1, 0) + at(1, 2) * m.at(1, 1) + at(2, 2)  * m.at(1, 2) + at(3, 2) * m.at(1, 3),
        at(0, 2) * m.at(2, 0) + at(1, 2) * m.at(2, 1) + at(2, 2)  * m.at(2, 2) + at(3, 2) * m.at(2, 3),
        at(0, 2) * m.at(3, 0) + at(1, 2) * m.at(3, 1) + at(2, 2)  * m.at(3, 2) + at(3, 2) * m.at(3, 3),

        at(0, 3) * m.at(0, 0) + at(1, 3) * m.at(0, 1) + at(2, 3)  * m.at(0, 2) + at(3, 3) * m.at(0, 3),
        at(0, 3) * m.at(1, 0) + at(1, 3) * m.at(1, 1) + at(2, 3)  * m.at(1, 2) + at(3, 3) * m.at(1, 3),
        at(0, 3) * m.at(2, 0) + at(1, 3) * m.at(2, 1) + at(2, 3)  * m.at(2, 2) + at(3, 3) * m.at(2, 3),
        at(0, 3) * m.at(3, 0) + at(1, 3) * m.at(3, 1) + at(2, 3)  * m.at(3, 2) + at(3, 3) * m.at(3, 3)
      }};
    }

  private:
    std::array<double, 4 * 4> m_data;
};

Mat4 constructModelMatrix(const Vec3f& t, double a) {
  Mat4 m{{
    cos(a), 0.0, sin(a), t.x,
    0.0, 1.0, 0.0, t.y,
    -sin(a), 0.0, cos(a), t.z,
    0.0, 0.0, 0.0, 1.0
  }};

  return m;
}

Mat4 constructViewMatrix() {
  Mat4 m;

  // TODO

  return m;
}

Mat4 constructProjectionMatrix(double fovY, double aspect, double near, double far) {
  Mat4 m;
  const double fovX = aspect * fovY;
  const double t = near * tan(fovY * 0.5);
  const double b = -t;
  const double r = near * tan(fovX * 0.5);
  const double l = -r;

  m.set(0, 0, 2.0 * near / (r - l));
  m.set(2, 0, -(r + l) / (r - l));
  m.set(1, 1, 2.0 * near / (t - b));
  m.set(2, 1, (t + b) / (t - b));
  m.set(2, 2, -(far + near) / (far - near));
  m.set(3, 2, -2.0 * far * near / (far - near));
  m.set(2, 3, -1.0);
  m.set(3, 3, 0.0);

  return m;
}

std::vector<Vec4f> loadModel(const std::string& path) {
  std::ifstream stream(path);
  if (!stream.good()) {
    throw std::runtime_error(std::string("Could not open file '") + path + "'");
  }

  std::vector<Vec4f> model;
  char vChar = '_';
  std::stringstream ss;
  Vec4f vertex;

  while (!stream.eof()) {
    std::string line;
    std::getline(stream, line);

    ss.str(line);
    ss.clear();
    ss.seekg(0);
    vChar = '_';

    ss >> vChar;
    if (vChar != 'v') {
      break;
    }

    ss >> vertex.x;
    ss >> vertex.y;
    ss >> vertex.z;
    vertex.w = 1.0;

    model.push_back(std::move(vertex));
  }

  return model;
}

void drawScene(const std::vector<Vec2f>& projection) {
  const uint32_t w = 120;
  const uint32_t h = 40;

  std::array<int, w * h> bitmap;
  memset(bitmap.data(), 0, bitmap.size() * sizeof(int));

  for (auto pt : projection) {
    uint32_t x = static_cast<uint32_t>(w * ((pt.x + 1.0) / 2.0) + 0.5);
    uint32_t y = static_cast<uint32_t>(h * ((pt.y + 1.0) / 2.0) + 0.5);

    if (x < w && y < h) {
      bitmap[y * w + x] = 1;
    }
  }

  system("CLS");

  for (int j = h - 1; j >= 0; --j) {
    for (uint32_t i = 0; i < w; ++i) {
      int px = bitmap[j * w + i];
      std::cout << (px == 1 ? '*' : ' ');
    }
    std::cout << "\n";
  }
}

int main() {
  try {
    const auto dt = std::chrono::milliseconds{100};
    const double da = 0.1 * PI;
    double angle = 0.0;
    double xOffset = 0.0;
    double yOffset = -2.0;
    double zOffset = -8.0;
    Mat4 modelTransform = constructModelMatrix(Vec3f{ xOffset, yOffset, zOffset }, angle);
    Mat4 viewTransform = constructViewMatrix();
    Mat4 projectionTransform = constructProjectionMatrix(45.0, 1.0, 0.1, 100.0);

    std::vector<Vec4f> model = loadModel("teapot.obj");

    while (true) {
      std::vector<Vec2f> projection;
      for (auto& v : model) {
        Vec4f p = projectionTransform * (viewTransform * (modelTransform * v));
        Vec3f p_{ p.x / p.w, p.y / p.w, p.z / p.w };

        if (p_.x >= -1.0 && p_.x <= 1.0
          && p_.y >= -1.0 && p_.y <= 1.0
          && p_.z >= -1.0 && p_.z <= 1.0) {

          projection.push_back(Vec2f{p_.x, p_.y});
        }
      }

      drawScene(projection);

      std::this_thread::sleep_for(dt);

      angle += da;
      modelTransform = constructModelMatrix(Vec3f{ xOffset, yOffset, zOffset }, angle);
    }
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
