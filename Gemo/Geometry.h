/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */
///////////////////////////////////////////////////////////////////////

/**
 *Copyright:
 *This code is free software; you can redistribute it and/or modify
 *it under the terms of the GPL-3.0
 *
 * @author zhangjian
 * @date 2018.04
 *
 */

///////////////////////////////////////////////////////////////////////

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <iostream>
#include <cmath>
using namespace std;

//这个文件里面声明四个类
//点类（矢量）
class Vector2D
{
public:
    static const double error;
    static const Vector2D INVALIDATED;  //用来处理无交点
private:
    double M_x;
    double M_y;

public:
    Vector2D(double x1=0.0,double y1=0.0):M_x(x1),M_y(y1) {}
    //重载<<运算符，方便输出
    friend const ostream& operator<<(const ostream& out,const Vector2D& c)
    {
        if(c.M_x==error&&c.M_y==error) cout<<"点不存在！"<<endl;
        else
        cout<<"Vector("<<c.M_x<<","<<c.M_y<<")"<<endl;
        return out;
    }

    Vector2D & assign( const double & xx,const double & yy )
    {
       M_x = xx;
       M_y = yy;
       return *this;
    }

    double x() const { return M_x;}
    double y() const { return M_y;}

    bool isValid() const
    {
      return ( ( M_x != error ) && ( M_y != error ) );
    }

    virtual ~Vector2D(){}

};



//直线类
class Line2D {
public:
    static const double ERROR;
private:

    double M_a;
    double M_b;
    double M_c;

    Line2D();
public:
    Line2D( const double & a,const double & b,const double & c )
        : M_a( a ), M_b( b ), M_c( c ) { }

    Line2D( const Vector2D & p1,const Vector2D & p2 )
    {
        M_a = -( p2.y() - p1.y() );
        M_b = p2.x() - p1.x();
        M_c = -M_a * p1.x() - M_b * p1.y();
    }

    double a() const { return M_a; }
    double b() const { return M_b; }
    double c() const { return M_c; }

    double getX( const double & y ) const
    {
        if ( fabs( M_a ) < 1.0e-10 )
        {
            return ERROR;
        }
        return -( M_b * y + M_c ) / M_a;
    }
    //求交点
    static Vector2D intersection( const Line2D & line1,const Line2D & line2 );

    Vector2D intersection( const Line2D & line ) const
    {
      return intersection( *this, line );
    }


};

//圆类
class Circle2D
{
private:
    Vector2D M_center;
    double M_radius;
public:
    Circle2D( const Vector2D & c,const double & r )
        : M_center( c ), M_radius( r )
    {
        if ( r < 0.0 )
        {
              std::cerr << "Circle2D::Circle2D(). radius must be positive value."
                        << std::endl;
              M_radius = 0.0;
        }
    }

    int intersection( const Line2D & line,Vector2D & sol1,
                  Vector2D & sol2 ) const;
};

//矩形类
class Rect2D
{
private:
    Vector2D top_left;      //两点确定一个矩形
    Vector2D bottom_right;

public:
    Rect2D():top_left(0.0,0.0),bottom_right(0.0,0.0){}
    Rect2D(Vector2D & c,Vector2D & d)
        :top_left(c.x(),c.y()),bottom_right(d.x(),d.y())
        {}
    //求四条边界
    Line2D leftEdge() const
    {
        return Line2D(top_left,Vector2D(top_left.x(),bottom_right.y()));
    }


    Line2D rightEdge() const
    {
        return Line2D( Vector2D(bottom_right.x(),top_left.y()), bottom_right );
    }

    Line2D topEdge() const
    {
        return Line2D( top_left, Vector2D(bottom_right.x(),top_left.y()) );
    }

    Line2D bottomEdge() const
    {
        return Line2D(Vector2D(top_left.x(),bottom_right.y()),bottom_right);
    }

    int intersection( const Line2D & line,
                  Vector2D & sol1,
                  Vector2D & sol2 ) const;
};



#endif // GEOMETRY_H
