#include "Common.h"

using namespace std;

// Этот файл сдаётся на проверку
// Здесь напишите реализацию необходимых классов-потомков `IShape`
class Rectangle : public IShape {
public:
    Rectangle(const Point& point, const Size& size,
              const shared_ptr<ITexture>& texture)
        : position_(point), size_(size), texture_(texture) {}

    Rectangle()
        : position_(), size_(), texture_() {}

    unique_ptr<IShape> Clone() const override {
        return make_unique<Rectangle>(position_, size_, texture_);
    }

    void SetPosition(Point p) override {
        position_ = move(p);
    }

    Point GetPosition() const override {
        return position_;
    }

    void SetSize(Size size) override {
        size_ = move(size);
    }

    Size GetSize() const override {
        return size_;
    }

    void SetTexture(shared_ptr<ITexture> texture) override {
        texture_ = move(texture);
    }

    ITexture* GetTexture() const override {
        return texture_.get();
    }

    void Draw(Image& img) const override {
        // количество пикселей по координате x, которые необходимо закрасить
        // '.' по умолчанию
        auto xFill = min<int>(img[0].size() - position_.x, size_.width);

        // то же для координаты y
        auto yFill = min<int>(img.size() - position_.y, size_.height);

        for(auto it = img.begin() + position_.y, itEnd = it + yFill;
            it != itEnd; ++it) {
            auto strItBegin = it->begin() + position_.x;
            auto strItEnd = strItBegin + xFill;
            fill(strItBegin, strItEnd, '.');
        }

        if(texture_) {
            xFill = min(xFill, texture_->GetSize().width);
            yFill = min(yFill, texture_->GetSize().height);

            const auto& textureImg = texture_->GetImage();

            for(int col = 0, I = yFill; col < I; ++col) {
                for(int row = 0, J = xFill; row < J; ++row) {
                    img[col + position_.y][row + position_.x] = textureImg[col][row];
                }
            }
        }
    }

private:
    Point position_;
    Size size_;
    shared_ptr<ITexture> texture_;
};

class Ellipse : public IShape {
public:
    Ellipse(const Point& point, const Size& size,
            const shared_ptr<ITexture>& texture)
        : position_(point), size_(size), texture_(texture) {}

    Ellipse()
        : position_(), size_(), texture_() {}

    unique_ptr<IShape> Clone() const override {
        return make_unique<Ellipse>(position_, size_, texture_);
    }

    void SetPosition(Point p) override {
        position_ = move(p);
    }

    Point GetPosition() const override {
        return position_;
    }

    void SetSize(Size size) override {
        size_ = move(size);
    }

    Size GetSize() const override {
        return size_;
    }

    void SetTexture(shared_ptr<ITexture> texture) override {
        texture_ = move(texture);
    }

    ITexture* GetTexture() const override {
        return texture_.get();
    }

    void Draw(Image& img) const override {
        // заливка по умолчанию
        // размеры ограничивающего прямоугольника, вписанного в картинку Image
        // координата x
        auto xBorder = min<int>(img[0].size() - position_.x, size_.width);
        // координата y
        auto yBorder = min<int>(img.size() - position_.y, size_.height);

        for(auto col = 0, I = yBorder; col < I; ++col) {
            for(auto row = 0, J = xBorder; row < J; ++row) {
                if(IsPointInEllipse({row, col}, size_)) {
                    img[col + position_.y][row + position_.x] = '.';
                }
            }
        }

        if(texture_) {
            // ограниичивающий прямоугольник у текстуры
            // координата x
            xBorder = min(xBorder, texture_->GetSize().width);
            // координата y
            yBorder = min(yBorder, texture_->GetSize().height);

            const auto& textureImg = texture_->GetImage();

            for(int col = 0, I = yBorder; col < I; ++col) {
                for(int row = 0, J = xBorder; row < J; ++row) {
                    if(IsPointInEllipse({row, col}, size_)) {
                        img[col + position_.y][row + position_.x] = textureImg[col][row];
                    }
                }
            }
        }
    }

private:
    Point position_;
    Size size_;
    shared_ptr<ITexture> texture_;
};


// Напишите реализацию функции
unique_ptr<IShape> MakeShape(ShapeType shape_type) {
    unique_ptr<IShape> shape;

    switch(shape_type) {
    case ShapeType::Rectangle:
        shape = move(make_unique<Rectangle>());
        break;
    case ShapeType::Ellipse:
        shape = move(make_unique<Ellipse>());
        break;
    }

   return shape;
}
