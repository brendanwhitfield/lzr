
#pragma once

#include <vector>
#include <cmath>
#include <lzr.h>

namespace lzr {



//constants
#define PI 3.14159265358979323846
#define ANGLE_ANY         4.0 //an invalid point angle denoting no angle preference

//functions
#define ANGLE(a, b)       ( std::atan2(b.y - a.y, b.x - a.y) * -1 ) //the angle from point A to point B on range (-PI, PI] in screen coords
#define ANGLE_NORM(a)     ( std::fmod(a, PI) ) //constrains angles to (-PI, PI]
#define ANGLE_OPPOSITE(a) ( ANGLE_NORM(a + PI) )

/*
    The angle formed between three points. Because of the way point
    angles are stored, only points B and C are neccessary.
    (see fill_angle() for details)

              C .
               /
              /
    A      B /
    ._______.

    This macros reports the angle at which the figure DEVIATES from a
    straight line:

                .
               /
              /
             /  ) theta
    ._______.  .  .  .  .

*/
#define ANGLE_FORMED(b, c) (std::abs(c.angle - b.angle))



/*
 * specialized point class for handling the endpoints of a path
 */
class Optimizer_Point
{
public:
    Point point;  //copy of actual point data
    double angle; //the angle for entering the path at this point (radians)
};



/*
 * struct defining a continguous segment of laser light
 */
class Optimizer_Path
{
public:
    Optimizer_Path(size_t _a, size_t _b, const std::vector<Optimizer_Point>* _points);

    size_t a;   //index of the front point
    size_t b;   //index of the back point
    bool cycle; //whether or not this path is cyclic (EXPERIMENTAL)

    size_t size();
    void invert();
    double entrance_angle();
    double exit_angle();
    Optimizer_Point front();
    Optimizer_Point back();
    Optimizer_Point operator[](size_t n);

private:
    //save a pointer to the points that this path references
    const std::vector<Optimizer_Point>* points;
};



/*
 * private optimizer context
 */
class Optimizer_Internals
{
public:
    //main optimization function. Accepts pointer for
    //the wrapper/settings holder object.
    int run(Optimizer* settings, Frame& frame);

private:
    //main frame buffers
    std::vector<Optimizer_Point> points; //point buffer
    std::vector<Optimizer_Path> paths;   //path buffer

    //state, preserved between frames
    Optimizer_Point last_known_point; //the last point from the previously optimized frame
    size_t num_last_known_anchors;    //number of ending anchor points on the previous frame



    //Top level functions (these are order specific)

    /*
        Splits a point buffer into individual path segments.
        Loads the result into the given path buffer.
    */
    void find_paths(Optimizer* settings);

    void fill_angle();
    void path_split(double split_angle);
    void fill_cycle(double split_angle);


    /*
        Arranges the paths in the path buffer to minimize blank time,
        and route the laser in a sensical path.
    */
    void reorder_paths(Optimizer* settings);

    double cost(Optimizer_Path a, Optimizer_Path b);
    void swap_paths(size_t a, size_t b);
    void find_next_and_swap(size_t start, Optimizer_Point laser);


    /*
        Fills the given output buffer according to the modified path buffer.
        Inserts/generates new blanking jumps. Returns the new size of the
        output buffer.
    */
    void compile_paths(Optimizer* settings, Frame& frame);

    size_t num_beginning_anchors(Optimizer_Path path);
    void add_path_to_frame(Optimizer* settings,
                           Frame& frame,
                           Optimizer_Path path);
    void blank_between(Optimizer* settings,
                       Frame& frame,
                       Point a,
                       Point b);
};


} // namespace lzr
