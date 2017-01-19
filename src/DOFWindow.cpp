#include "DOFWindow.hpp"
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>

#include <FL/Fl_Scroll.H>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>

int dmp::DOFSlider::handle(int event)
{
  switch (event)
    {
    case FL_DRAG:
      if (Fl::event_button() == FL_LEFT_MOUSE)
        {
          if (node != nullptr)
            {
              float * val = nullptr;
              switch (axis)
                {
                case X:
                  val = &(node->posex);
                  break;
                case Y:
                  val = &(node->posey);
                  break;
                case Z:
                  val = &(node->posez);
                  break;
                }
              *val = (float) value();
              node->rotateDirty = true;
            }
        }
    }
  return Fl_Hor_Value_Slider::handle(event);
}

static double clampFloat(float val)
{
  // FLTK seems to choke on +/- infinity. Since a 1 inch slider that
  // ranges between +/- infinity is probably not useful anyway, let's
  // clamp it to +/- (2 * PI)
  return (double) glm::clamp(val,
                             glm::pi<float>() * -2.0f,
                             glm::pi<float>() * 2.0f);
}

int dmp::DOFWindow::addSkelNode(Balljoint * node, int offset)
{
  auto box = new Fl_Box(0, offset, 150, 100, node->name.c_str());
  box->align(FL_ALIGN_WRAP);
  auto rotx = new DOFSlider(150, offset, 150, 30, "x rotation");
  rotx->node = node;
  rotx->axis = X;
  rotx->bounds(clampFloat(node->rotxmin), clampFloat(node->rotxmax));
  rotx->value((double)node->posex);
  rotx->align(FL_ALIGN_RIGHT);
  auto roty = new DOFSlider(150, offset + 30, 150, 30, "y rotation");
  roty->node = node;
  roty->axis = Y;
  roty->bounds(clampFloat(node->rotymin), clampFloat(node->rotymax));
  roty->value((double)node->posey);
  roty->align(FL_ALIGN_RIGHT);
  auto rotz = new DOFSlider(150, offset + 60, 150, 30, "z rotation");
  rotz->node = node;
  rotz->axis = Z;
  rotz->bounds(clampFloat(node->rotzmin), clampFloat(node->rotzmax));
  rotz->value((double)node->posez);
  rotz->align(FL_ALIGN_RIGHT);

  auto outOffset = offset;
  for (auto & curr : node->children)
    {
      outOffset = addSkelNode(curr.get(), outOffset + 100);
    }
  return outOffset;
}

dmp::DOFWindow::DOFWindow(Balljoint * skel)
{
  mWindow = std::make_unique<Fl_Window>(400, 720);
  mWindow->label("DOFs");
  auto scroll = new Fl_Scroll(0, 0, 400, 720);
  scroll->begin();
  addSkelNode(skel, 0);

  scroll->end();
  mWindow->end();
}

void dmp::DOFWindow::show()
{
  expect("DOF window not null", mWindow);
  mWindow->show();
}

void dmp::DOFWindow::pollEvents()
{
  Fl::check();
}
