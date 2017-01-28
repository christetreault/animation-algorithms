#ifndef DMP_DOFWINDOW_HPP
#define DMP_DOFWINDOW_HPP

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hor_Value_Slider.H>

#include "Scene/Model/Skeleton.hpp"

namespace dmp
{
  class DOFWindow
  {
  public:
    DOFWindow() = delete;
    DOFWindow(const DOFWindow &) = delete;
    DOFWindow & operator=(const DOFWindow &) = delete;
    DOFWindow(DOFWindow &&) = delete;
    DOFWindow & operator=(DOFWindow &&) = delete;

    ~DOFWindow() {}

    DOFWindow(Balljoint * skel);
    void show();
    void pollEvents();
  private:
    int addSkelNode(Balljoint *, int offset);
    std::unique_ptr<Fl_Window> mWindow = nullptr;
  };

  enum Axis {X, Y, Z};

  class DOFSlider : public Fl_Hor_Value_Slider
  {
  public:
    DOFSlider(int x, int y, int w, int h, const char *label = 0)
      : Fl_Hor_Value_Slider(x, y, w, h, label) {}
    int handle(int event) override;
    Balljoint * node = nullptr;
    Axis axis;
  };
}

#endif
