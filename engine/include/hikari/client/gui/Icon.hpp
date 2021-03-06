#ifndef HIKARI_CLIENT_GUI_ICON
#define HIKARI_CLIENT_GUI_ICON

#include <guichan/widgets/icon.hpp>
#include <guichan/rectangle.hpp>

namespace hikari {
namespace gui {

    class Icon : public gcn::Icon {
    private:
        gcn::Rectangle imageSubrectangle;
        bool opaqueFlag;
        int offsetX;
        int offsetY;

    public:
        /**
         * Default constructor.
         */
        Icon();

        /**
         * Constructor.
         *
         * @param filename The filename of the image to display.
         */
        Icon(const std::string& filename);

        /**
         * Constructor.
         *
         * @param image The image to display.
         */
        Icon(const gcn::Image* image);

        /**
         * Destructor.
         */
        virtual ~Icon();

        gcn::Rectangle getSubrectangle() const;

        void setSubrectangle(const gcn::Rectangle & rect);

        void setImageOffset(int x, int y);

        bool isOpaque() const;

        void setOpaque(bool opaque);

        // Inherited from Widget
        virtual void draw(gcn::Graphics* graphics);

        virtual void _draw(gcn::Graphics* graphics);
    };

} // hikari::gui
} // hikari

#endif // HIKARI_CLIENT_GUI_ICON