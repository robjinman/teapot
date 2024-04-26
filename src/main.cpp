#include "teapot.hpp"
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <array>
#include <chrono>
#include <thread>
#include <cmath>
#include <cstring>
#include <cassert>
#include <map>

#undef near
#undef far

const double PI = 3.14159265359;

class WinIO {
  public:
    WinIO(int width, int height)
      : m_hasClosed(false)
      , m_width(width)
      , m_height(height) {

      m_hInstance = GetModuleHandle(NULL);

      WNDCLASSEX wcex{};
      wcex.cbSize = sizeof(WNDCLASSEX);
      wcex.style = CS_HREDRAW | CS_VREDRAW;
      wcex.lpfnWndProc = WinIO::wndProc;
      wcex.cbClsExtra = 0;
      wcex.cbWndExtra = 0;
      wcex.hInstance = m_hInstance;
      wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
      wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
      wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
      wcex.lpszMenuName = NULL;
      wcex.lpszClassName = ClassName;
      wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

      if (!RegisterClassEx(&wcex)) {
        throw std::runtime_error("Error creating window; Failed to register window class");
      }

      const TCHAR title[] = _T("Teapot");

      m_hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, ClassName, title,
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, m_width, m_height, NULL,
        NULL, m_hInstance, NULL);

      if (!m_hWnd) {
        throw std::runtime_error("Error creating window");
      }

      ShowWindow(m_hWnd, SW_SHOW);
      UpdateWindow(m_hWnd);
      SetForegroundWindow(m_hWnd);
      SetFocus(m_hWnd);

      m_bitmapInfo.bmiHeader.biSize = sizeof(m_bitmapInfo.bmiHeader);
      m_bitmapInfo.bmiHeader.biWidth = m_width;
      m_bitmapInfo.bmiHeader.biHeight = m_height;
      m_bitmapInfo.bmiHeader.biPlanes = 1;
      m_bitmapInfo.bmiHeader.biBitCount = 32;
      m_bitmapInfo.bmiHeader.biCompression = BI_RGB;
      m_bitmapInfo.bmiHeader.biSizeImage = 0;
      m_bitmapInfo.bmiHeader.biXPelsPerMeter = 1;
      m_bitmapInfo.bmiHeader.biYPelsPerMeter = 1;
      m_bitmapInfo.bmiHeader.biClrUsed = 0;
      m_bitmapInfo.bmiHeader.biClrImportant = 0;

      m_bitmap = CreateCompatibleBitmap(GetDC(m_hWnd), m_width, m_height);
      m_bitmapDc = CreateCompatibleDC(GetDC(m_hWnd));
      SelectObject(m_bitmapDc, m_bitmap);

      m_instances[m_hWnd] = this;
    }

    bool update() {
      MSG msg;

      InvalidateRect(m_hWnd, NULL, FALSE);

      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

      return !m_hasClosed;
    }

    void setImage(const void* data) {
      SetDIBits(m_bitmapDc, m_bitmap, 0, m_height, data, &m_bitmapInfo, DIB_RGB_COLORS);
    }

    ~WinIO() {
      if (m_hWnd) {
        m_instances.erase(m_hWnd);
        DestroyWindow(m_hWnd);
      }

      UnregisterClass(ClassName, m_hInstance);
    }

  private:
    static TCHAR ClassName[];

    bool m_hasClosed;
    HWND m_hWnd;
    HINSTANCE m_hInstance;
    int m_width;
    int m_height;
    BITMAPINFO m_bitmapInfo;
    HBITMAP m_bitmap;
    HDC m_bitmapDc;

    static std::map<HWND, WinIO*> m_instances;
    static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void render() {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(m_hWnd, &ps);

      BitBlt(hdc, 0, 0, m_width, m_height, m_bitmapDc, 0, 0, SRCCOPY);

      EndPaint(m_hWnd, &ps);
    }

    void onClose() {
      m_hasClosed = true;
    }
};

TCHAR WinIO::ClassName[] = _T("WinIO");
std::map<HWND, WinIO*> WinIO::m_instances{};

LRESULT CALLBACK WinIO::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (m_instances.count(hWnd)) {
    WinIO& window = *m_instances.at(hWnd);

    switch (uMsg) {
      case WM_PAINT: {
        window.render();
        return 0;
      }
      case WM_CLOSE: {
        window.onClose();
      }
      default: {}
    }
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

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

struct Bitmap {
  Bitmap(int width, int height)
    : width(width)
    , height(height)
    , data(width * height * 4) {}

  uint32_t width;
  uint32_t height;
  std::vector<uint8_t> data;

  void clear() {
    memset(data.data(), 0, width * height * 4);
  }

  void setPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
    data[y * width * 4 + x * 4 + 0] = b;
    data[y * width * 4 + x * 4 + 1] = g;
    data[y * width * 4 + x * 4 + 2] = r;
    data[y * width * 4 + x * 4 + 3] = 1;
  }
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
  m.set(2, 0, (r + l) / (r - l));
  m.set(1, 1, 2.0 * near / (t - b));
  m.set(2, 1, (t + b) / (t - b));
  m.set(2, 2, -(far + near) / (far - near));
  m.set(3, 2, -2.0 * far * near / (far - near));
  m.set(2, 3, -1.0);
  m.set(3, 3, 0.0);

  return m;
}

void drawScene(const std::vector<Vec2f>& projection, Bitmap& canvas) {
  canvas.clear();

  for (auto pt : projection) {
    uint32_t x = static_cast<uint32_t>(canvas.width * ((pt.x + 1.0) / 2.0) + 0.5);
    uint32_t y = static_cast<uint32_t>(canvas.height * ((pt.y + 1.0) / 2.0) + 0.5);

    if (x < canvas.width && y < canvas.height) {
      canvas.setPixel(x, y, 0, 255, 0);
    }
  }
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  const int width = 640;
  const int height = 480;

  try {
    WinIO window(width, height);

    const long fps = 30;
    const double revsPerSecond = 1.0;
    const auto dt = std::chrono::milliseconds{1000 / fps};
    const double da = (2.0 * PI / fps) * revsPerSecond;
    double angle = 0.0;
    double xOffset = 0.0;
    double yOffset = -2.0;
    double zOffset = -8.0;
    Mat4 modelTransform = constructModelMatrix(Vec3f{ xOffset, yOffset, zOffset }, angle);
    Mat4 viewTransform = constructViewMatrix();
    Mat4 projectionTransform = constructProjectionMatrix(45.0, 1.0, 0.1, 100.0);

    Bitmap canvas(width, height);

    assert(teapot.size() % 3 == 0);

    while (window.update()) {
      std::vector<Vec2f> projection;
      for (size_t i = 0; i < teapot.size(); i += 3) {
        Vec4f v{teapot[i + 0], teapot[i + 1], teapot[i + 2], 1};
        Vec4f p = projectionTransform * viewTransform * modelTransform * v;
        Vec3f p_{ p.x / p.w, p.y / p.w, p.z / p.w };

        if (p_.x >= -1.0 && p_.x <= 1.0
          && p_.y >= -1.0 && p_.y <= 1.0
          && p_.z >= -1.0 && p_.z <= 1.0) {

          projection.push_back(Vec2f{p_.x, p_.y});
        }
      }

      drawScene(projection, canvas);
      window.setImage(canvas.data.data());

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
