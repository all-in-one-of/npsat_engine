#ifndef CGAL_FUNCTIONS_H
#define CGAL_FUNCTIONS_H
#include <iostream>
#include <deal.II/base/point.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

// includes from AABB tree search
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

// includes from scatter interpolation
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Interpolation_traits_2.h>
#include <CGAL/natural_neighbor_coordinates_2.h>
#include <CGAL/interpolation_functions.h>

// includes from xyz class
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Point_set_2.h>

// includes for particle tracking searching functions
#include <CGAL/Range_segment_tree_traits.h>
#include <CGAL/Range_tree_k.h>

// includes for polyxpoly
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Segment_2.h>

// includes for barycentric coordinates
#include <CGAL/Barycentric_coordinates_2/Wachspress_2.h>
#include <CGAL/Barycentric_coordinates_2/Discrete_harmonic_2.h>
#include <CGAL/Barycentric_coordinates_2/Mean_value_2.h>
#include <CGAL/Barycentric_coordinates_2/Generalized_barycentric_coordinates_2.h>



/*! \file cgal_functions.h
    \brief CGAL types and functions.

    This include file contains all type definitions and methods that are based on the CGAL
    library.
*/


/*! \var typedef CGAL::Exact_predicates_inexact_constructions_kernel ine_Kernel
    \brief Definition of inexact kernel
*/
typedef CGAL::Exact_predicates_inexact_constructions_kernel                 ine_Kernel;

/*! \var typedef ine_Kernel::Point_3 ine_Point3
    \brief Definition type of a 3D point based on inexact kernel
*/
typedef ine_Kernel::Point_3                                                 ine_Point3;

/*! \var typedef ine_Kernel::Triangle_3 ine_Triangle;
    \brief Type definition of a triangle based on inexact kernel
*/
typedef ine_Kernel::Triangle_3                                              ine_Triangle;

/*! \var  typedef std::list<ine_Triangle>  ineTriangle_list
    \brief Type definition of a list of triangles based on inexact kernel
*/
typedef std::list<ine_Triangle>                                             ineTriangle_list;

// typedefs from AABB tree
typedef std::list<ine_Triangle>::iterator                                   ine_Iterator;
typedef CGAL::AABB_triangle_primitive<ine_Kernel, ine_Iterator>             ine_Primitive;
typedef CGAL::AABB_traits<ine_Kernel, ine_Primitive>                        ine_AABB_triangle_traits;
typedef CGAL::AABB_tree<ine_AABB_triangle_traits>                           ine_Tree;
typedef ine_Tree::Primitive_id                                              ine_primitive_id;

// typedefs from scatter interpolation
typedef CGAL::Delaunay_triangulation_2<ine_Kernel>                          ine_Delaunay_triangulation;
typedef ine_Kernel::FT                                                      ine_Coord_type;
typedef ine_Kernel::Point_2                                                 ine_Point2;

//typedefs for xyz class
typedef CGAL::Triangulation_vertex_base_with_info_2<unsigned, ine_Kernel>   Vb;
typedef CGAL::Triangulation_data_structure_2<Vb>                            Tds;
typedef CGAL::Point_set_2<ine_Kernel,Tds>::Vertex_handle                    Vertex_handle;
typedef CGAL::Point_set_2<ine_Kernel,Tds>                                   PointSet2;

//typedefs for particle tracking
typedef CGAL::Range_tree_map_traits_3<ine_Kernel, int>                      ine_dDTraits;
typedef CGAL::Range_tree_3<ine_dDTraits>                                    Range_tree_3_type;
typedef ine_dDTraits::Key                                                   ine_Key;
typedef ine_dDTraits::Interval                                              ine_Interval;
typedef ine_dDTraits::Pure_key                                              ine_Pure_key;

typedef CGAL::Exact_predicates_exact_constructions_kernel                   exa_Kernel;
typedef exa_Kernel::Point_2                                                 exa_Point2;
typedef CGAL::Polygon_2<exa_Kernel>                                         Polygon_2;

//typedefs for Barycentric
//typedef CGAL::Barycentric_coordinates::Wachspress_2<exa_Kernel> Wachspress;
//typedef CGAL::Barycentric_coordinates::Generalized_barycentric_coordinates_2<Wachspress, exa_Kernel> Wachspress_coordinates;
//typedef CGAL::Barycentric_coordinates::Discrete_harmonic_2<exa_Kernel> Discrete_harmonic;
//typedef CGAL::Barycentric_coordinates::Generalized_barycentric_coordinates_2<Discrete_harmonic, exa_Kernel> Discrete_harmonic_coordinates;
typedef CGAL::Barycentric_coordinates::Mean_value_2<ine_Kernel> Mean_value;
typedef CGAL::Barycentric_coordinates::Generalized_barycentric_coordinates_2<Mean_value, ine_Kernel> Mean_value_coordinates;


void find_intersection_inner(ine_Tree& tree,
                             ineTriangle_list& triangle_list,
                             ine_Triangle& triangle_query,
                             std::vector<int>& ids){

    std::list<ine_primitive_id> intersects;
    tree.all_intersected_primitives(triangle_query, std::back_inserter(intersects));
    for (std::list<ine_primitive_id>::iterator it = intersects.begin(); it != intersects.end(); it++){
        ine_primitive_id itt = *it;
        std::size_t index_in_vector = static_cast<std::size_t>(std::distance(triangle_list.begin(), itt));
        ids.push_back(static_cast<int>(index_in_vector));
    }
}

/*!
 * \brief The find_intersection_in_AABB_TREE is a fast way to compute intersections between
 * triangulation cells and stream areas.
 *
 * The function splits the quadrilateral into 2 triangles, and for each triangle identifies
 * which river segments intersect and returns their ids
 *
 * \param tree structure holds the data of the stream outlines
 * \param triangle_list
 * \param xp are the x coordinates of the quadrilateral cells
 * \param yp are the x coordinates of the quadrilateral cells
 * \param ids are the ids of the river segments that intersects with the quadrilateral cell in question
 */
bool find_intersection_in_AABB_TREE(ine_Tree& tree,
                                    ineTriangle_list& triangle_list,
                                    std::vector<double>& xp,
                                    std::vector<double>& yp,
                                    std::vector<int>& ids){
    ids.clear();

    // This function expects quadrilaterals
    if (xp.size() != 4 || yp.size() != 4)
        return false;

    // construct query triangles
    ine_Triangle tria1(ine_Point3(xp[0], yp[0], 0.0),
                       ine_Point3(xp[1], yp[1], 0.0),
                       ine_Point3(xp[2], yp[2], 0.0));
    find_intersection_inner(tree, triangle_list, tria1, ids);
    ine_Triangle tria2(ine_Point3(xp[0], yp[0], 0.0),
                       ine_Point3(xp[2], yp[2], 0.0),
                       ine_Point3(xp[3], yp[3], 0.0));
    find_intersection_inner(tree, triangle_list, tria2, ids);

    if (ids.size() > 0)
        return true;
    else
        return false;
}

/*!
 * \brief scatter_2D_interpolation
 * \param DT is a triangulation of the 2D scattered points
 * \param function_values is an array of data to be interpolated. If the size of this array is 1, then it is assume that the set is 2D.
 * \param point is a deal 3D point
 * \return the interpolated value. The code starts by calculating normalized coordinates of the point we seek its value. Then the
 * normalized coordinates are used as many times as needed to perform the interpolation for each layer.

 */
double scatter_2D_interpolation(const ine_Delaunay_triangulation& DT, // const
                                const std::vector<std::map<ine_Point2, ine_Coord_type, ine_Kernel::Less_xy_2> >& function_values, //const
                                 dealii::Point<3>& point){

    typedef CGAL::Data_access< std::map<ine_Point2, ine_Coord_type, ine_Kernel::Less_xy_2 > > Value_access;
    // For some reason declaring this as unsigned long helps removing many warnings
    unsigned long Ndata = function_values.size();

    ine_Point2 p(point[0], point[1]);
    std::vector< std::pair< ine_Point2, ine_Coord_type > > coords;
    ine_Coord_type norm = CGAL::natural_neighbor_coordinates_2(DT, p, std::back_inserter(coords)).second;
    if (std::isnan(norm)){
        // jiggle the point
        int cnt = 0;
        while (true){
			std::cout << "try # " << cnt + 1 << std::endl;
            coords.clear();
            double xr = p[0] + 0.01*(-1.0 + 2.0*(static_cast<double>(rand())/static_cast<double>(RAND_MAX)));
            double yr = p[1] + 0.01*(-1.0 + 2.0*(static_cast<double>(rand())/static_cast<double>(RAND_MAX)));
            ine_Point2 p_try(xr, yr);
            norm = CGAL::natural_neighbor_coordinates_2(DT, p_try, std::back_inserter(coords)).second;
            if (!std::isnan(norm)){
                break;
			}
            else
                cnt++;
            if (cnt > 20)
                break;
        }
    }

    ine_Coord_type result = 0;

    if (Ndata == 1){
        result =  CGAL::linear_interpolation(coords.begin(), coords.end(), norm,
                                             Value_access(function_values[0]));
    }else{
        unsigned long Nlay = (Ndata + 1)/2;
        std::vector<double> z(Nlay - 1);
        std::vector<double> v(Nlay);
        bool is_data = true; unsigned long cnt_data = 0; unsigned long cnt_elev = 0;
        for (unsigned long i = 0; i < Ndata; ++i){
            if (is_data){
                v[cnt_data] =  static_cast<double>(CGAL::linear_interpolation(coords.begin(), coords.end(), norm,
                                                    Value_access(function_values[i])));
                is_data = false;
                cnt_data++;
            }else{
                z[cnt_elev] =  static_cast<double>(CGAL::linear_interpolation(coords.begin(), coords.end(), norm,
                                                    Value_access(function_values[i])));
                is_data = true;
                cnt_elev++;
            }
        }

        bool p_found = false;
        if (point[2] >= z[0]){
            result = v[0];
            p_found = true;
        }
        if (point[2] <= z[Nlay - 2] && !p_found){
            result = v[Nlay-1];
            p_found = true;
        }
        if (!p_found){

            // Constant cell interpolation
            /*
            for (unsigned int k = 0; k < z.size()-1; ++k){
                if (point[2] <= z[k] && point[2] > z[k+1])
                    result = v[k+1];
            }
            */

            // interpolate between cells
            for (unsigned int k = 0; k < z.size(); ++k){
                double mid_ztop, mid_zbot, val_top, val_bot;
                if (k == 0){
                    mid_ztop = z[k];
                    mid_zbot = (z[k] + z[k+1]) / 2.0;
                }
                else if (k == z.size()-1){
                    mid_ztop = (z[k-1] + z[k]) / 2.0;
                    mid_zbot = z[k];
                }
                else{
                    mid_ztop = (z[k-1] + z[k]) / 2.0;
                    mid_zbot = (z[k+1] + z[k]) / 2.0;
                }
                val_top = v[k];
                val_bot = v[k+1];

                if (point[2] <= mid_ztop && point[2] >= mid_zbot){
                    double u = (point[2] - mid_zbot)/(mid_ztop - mid_zbot);
                    result = val_bot*(1-u) + val_top*u;
                    break;
                }
            }
        }
    }
    if (std::isnan(result))
        std::cout << norm << "scatter_2D_interpolation will return NAN for Ndata=" << Ndata << " and p=(" << point[0] << "," << point[1] << "," << point[2] << ")" << std::endl;

    return result;
}

/*!
 * \brief get_point_ids_in_set Searches if any point of a given set of points lay within a rectangular area
 * \param Pset Is the Set of points
 * \param xp are the x coordinates of the rectangular area
 * \param yp are the y coordinates of the rectangular area
 * \param ids are the ids of the points found in the rectangular area
 * \return
 */
bool get_point_ids_in_set(PointSet2& Pset, std::vector<double>& xp, std::vector<double>& yp, std::vector<int>& ids){
    ids.clear();
    std::list<Vertex_handle> LV;

    if (xp.size() == 4 && yp.size() == 4){
        double xmin, xmax, ymin, ymax;
        xmin = 99999999999.9;
        xmax = -99999999999.9;
        ymin = 99999999999.9;
        ymax = -99999999999.9;
        for (unsigned int i = 0; i < xp.size(); ++i){
            if (xp[i] < xmin)
                xmin = xp[i];
            if (xp[i] > xmax)
                xmax = xp[i];
            if (yp[i] < ymin)
                ymin = yp[i];
            if (yp[i] > ymax)
                ymax = yp[i];
        }
        ine_Point2 p1(xmin, ymax);
        ine_Point2 p2(xmin, ymin);
        ine_Point2 p3(xmax, ymin);
        ine_Point2 p4(xmax, ymax);
        // a is the upper left point,
        // b the lower left,
        // c the lower right and
        // d the upper right point of the iso rectangle
        Pset.range_search(p1,p2,p3,p4, std::back_inserter(LV));
    }
    else if(xp.size() == 2 && yp.size() == 2){
        ine_Point2 p1(xp[0], yp[0]);
        ine_Point2 p2(xp[1], yp[1]);
        CGAL::Circle_2<ine_Kernel> rc(p1, p2,CGAL::Orientation::COUNTERCLOCKWISE);
        Pset.range_search(rc, std::back_inserter(LV));
    }
    else{
        return false;
    }

    if (LV.size() == 0){
        return false;
    }
    else{
        std::list<Vertex_handle>::const_iterator it = LV.begin();
        for (;it != LV.end(); ++it)
             ids.push_back(static_cast<int>((*it)->info()));
        return true;
    }
}

/*!
 * \brief circle_search_in_2DSet Searches if the point #p_in in question is closer than #r
 * to any other point in the set #PSet
 * \param Pset Is the Set of points
 * \param p_in is the point. Although the point is defined as 3D in practice the z coordinate is neglected
 * and the
 * \param r is the search distance
 * \return  a list of ids that correspond the the ids in the set #PSet of the points that lay inside the circle
 * with center #p_in and radius #r
 */
std::vector<int> circle_search_in_2DSet(PointSet2& PSet, ine_Point3 p_in, double r){
    std::vector<int> id;
    id.clear();

    CGAL::Circle_2<ine_Kernel> rc(ine_Point2(p_in.x(), p_in.y()), r);
    std::list<Vertex_handle> LV;
    PSet.range_search(rc, std::back_inserter(LV));
    if (LV.size() > 0){
        std::list<Vertex_handle>::const_iterator it = LV.begin();
        for (; it != LV.end(); ++it){
            id.push_back( static_cast<int>((*it)->info()) );
        }
    }
    return id;
}

bool polyXpoly(std::vector<double> X1, std::vector<double> Y1,
               std::vector<double> X2, std::vector<double> Y2){
    Polygon_2 P;
    for (unsigned int i = 0; i < X1.size(); ++i)
        P.push_back (exa_Point2 (X1[i], Y1[i]));

    Polygon_2 Q;
    for (unsigned int i = 0; i < X2.size(); ++i)
        Q.push_back (exa_Point2 (X2[i], Y2[i]));

    return CGAL::do_intersect (P, Q);
}

template <int dim>
std::vector<double> barycentricCoords(std::vector<double> xv, std::vector<double> yv, dealii::Point<dim> p){

    std::vector<ine_Point2> vertices;
    for (unsigned int i = 0; i < xv.size(); ++i)
        vertices.push_back(ine_Point2(xv[i], yv[i]));

    // Instantiate the class with Wachspress coordinates for the polygon defined above.
    //Wachspress_coordinates wachspress_coordinates(vertices.begin(), vertices.end());
    //Discrete_harmonic_coordinates discrete_harmonic_coordinates(vertices.begin(), vertices.end());
    Mean_value_coordinates mean_value_coordinates(vertices.begin(), vertices.end());
    ine_Point2 p_q(p[0], p[1]);
    std::vector<ine_Kernel::FT> coordinates;
    coordinates.reserve(1);
    //try {
    mean_value_coordinates(p_q, std::back_inserter(coordinates), CGAL::Barycentric_coordinates::UNSPECIFIED_LOCATION, CGAL::Barycentric_coordinates::PRECISE);
    //wachspress_coordinates(p_q, std::back_inserter(coordinates),CGAL::Barycentric_coordinates::UNSPECIFIED_LOCATION,CGAL::Barycentric_coordinates::PRECISE);
    //} catch (...) {
        //wachspress_coordinates(p_q, std::back_inserter(coordinates),CGAL::Barycentric_coordinates::UNSPECIFIED_LOCATION,CGAL::Barycentric_coordinates::FAST);
    //}

    std::vector<double> bcoords;
    for(unsigned int j = 0; j < xv.size(); ++j){
        bcoords.push_back(CGAL::to_double(coordinates[j]));
    }
    return bcoords;
}

template<int dim>
bool any_point_inside(Range_tree_3_type& Range_tree_3,
                           dealii::Point<dim> p1,
                           dealii::Point<dim> p2,
                           std::vector<int>& ids){

    bool outcome = false;
    std::vector<ine_Key> OutputList;

    ine_Interval win;
    if (dim == 2){
        ine_Key a = ine_Key(ine_Pure_key(p1[0], p1[1], -1), 100);
        ine_Key b = ine_Key(ine_Pure_key(p2[0], p2[1], 1), 101);
        win = ine_Interval(a, b);
    }
    else if (dim == 3){
        ine_Key a = ine_Key(ine_Pure_key(p1[0], p1[1], p1[2]), 100);
        ine_Key b = ine_Key(ine_Pure_key(p2[0], p2[1], p2[2]), 101);
        win = ine_Interval(a, b);
    }

    Range_tree_3.window_query(win, std::back_inserter(OutputList));
    if (OutputList.size() == 0){
        outcome = false;
    }
    else{
        std::vector<ine_Key>::iterator current = OutputList.begin();
        for (; current != OutputList.end(); ++current){
            ids.push_back(current->second);
        }
        outcome = true;
    }
    return outcome;
}


// The original file has two more functions to add

#endif // CGAL_FUNCTIONS_H
