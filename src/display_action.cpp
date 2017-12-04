#include <Arduino.h>
#include "display_action.hpp"
#include "display.hpp"

namespace Display {
  double ActionT::elapsedTime(void)
  {
    return m_elapsed_time;
  }

  bool ActionT::isFinished()
  {
    return m_finished;
  }

  void ActionT::setFinished(bool status)
  {
    m_finished = status;
  }

  /* === Transition === */

  void Action::SlideTransition::tick(DisplayT *display, double elapsed_time)
  {
    m_elapsed_time += elapsed_time;
    m_actionA->tick(display, elapsed_time);
    m_actionB->tick(display, elapsed_time);
    if (elapsedTime() > m_duration)
      setFinished();
  }

  void Action::SlideTransition::draw(DisplayT *display, Coords coords)
  {
    const int height = display->getDisplayHeight();
    const int offset_y_a = (elapsedTime() / m_duration) * height;
    const int offset_y_b = offset_y_a - height;

    m_actionA->draw(display, coords+Coords{0, offset_y_a});
    m_actionB->draw(display, coords+Coords{0, offset_y_b});
  }
}
