/*
CSCI 420
Assignment 3 Raytracer
Name: Duoduo Yu
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define MAX_TRIANGLES 2000
#define MAX_SPHERES 10
#define MAX_LIGHTS 10
#define BACKGROUND_COLOR_RGB 230

char *filename=0;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2
int mode=MODE_JPEG;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0

#define PI 3.141592653
#define ERROR_CONSTRAINT 0.0001  // used to exclude some corner cases

unsigned char buffer[HEIGHT][WIDTH][3];

struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

typedef struct _Triangle
{
  struct Vertex v[3];
} Triangle;

typedef struct _Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
} Sphere;

typedef struct _Light
{
  double position[3];
  double color[3];
} Light;

// the struct below is used to store the information of the intersection, later the information can be used for shading function
typedef struct _Intersection_Parameter {
    double position[3];  // intersection
    double direction[3];  // direction of the ray
    double normal[3];
    double color_diffuse[3];
    double color_specular[3];
    double shininess;
} IntersectionParameter;

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles=0;
int num_spheres=0;
int num_lights=0;

double camera_pos[3];  // camera position
double aspect_ratio;  // aspect ratio
double start_x;  // left bottom corner coordinate
double start_y;  // left bottom corner coordinate
double increment_x;
double increment_y;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

double offset = 0.0033;  // used for anti-aliasing. This value should be adjusted for different scenes. For spheres it can be large and for triangles it should be smaller.
int anti_aliasing_size = 8;  // the number of samplings
double weight = 1.0 / (double)anti_aliasing_size;  // weight of each sampling
// the filter below is used to offset the vertex of triangles and certer of spheres
// I used 8 offsets and finally merge 8 results together
double anti_aliasing_filter[8][4] =
   {{offset, offset, offset, weight},
    {offset, offset, -offset, weight},
    {offset, -offset, offset, weight},
    {offset, -offset, -offset, weight},
    {-offset, offset, offset, weight},
    {-offset, offset, -offset, weight},
    {-offset, -offset, offset, weight},
    {-offset, -offset, -offset, weight}};

// initialize a triplet
void initializeTriplet(double* direction, double x, double y, double z) {
    direction[0] = x;
    direction[1] = y;
    direction[2] = z;
}

// copy values of triplets
void copyTriplet(double* a, double* b) {
    for (int i = 0; i < 3; i++) {
        a[i] = b[i];
    }
}

// clamp the value to lower limit and upper limit
void clamp(double* array, double lowerLimit, double upperLimit) {
    for (int i = 0; i < 3; i++) {
        if (array[i] < lowerLimit) array[i] = lowerLimit;
        if (array[i] > upperLimit) array[i] = upperLimit;
    }
}

// square of distance
double distanceSquare(double* a, double* b) {
    double distance = 0;
    for (int i = 0; i < 3; i++) {
        distance += pow(a[i] - b[i], 2);
    }
    return distance;
}

// dot product of two vectors
double dotProduct(double* a, double* b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// cross product of two vectors
void vectorCrossProduct(double* A, double* B, double* R) {
    R[0] = A[1] * B[2] - A[2] * B[1];
    R[1] = A[2] * B[0] - A[0] * B[2];
    R[2] = A[0] * B[1] - A[1] * B[0];
}

// calculate the point with start point and direction of a line
void vectorEquation(double* start, double* direction, double* p, double t) {
    for (int i = 0; i < 3; i++) {
        p[i] = start[i] + direction[i] * t;
    }
}

// vector AB : B - A
void vectorFromTwoPoints(double* a, double* b, double* result) {
    for (int i = 0; i < 3; i++) {
        result[i] = b[i] - a[i];
    }
}

void normalize(double* p) {
    float a = sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
    p[0] = p[0] / a;
    p[1] = p[1] / a;
    p[2] = p[2] / a;
}

// get area of a triangle
double triangleArea(double* a, double* b, double* c) {
    double ab[3], ac[3], crossProduct[3];
    vectorFromTwoPoints(a, b, ab);
    vectorFromTwoPoints(a, c, ac);
    vectorCrossProduct(ab, ac, crossProduct);
    return 0.5 * sqrt(crossProduct[0] * crossProduct[0] + crossProduct[1] * crossProduct[1] + crossProduct[2] * crossProduct[2]);
}

// calculate point values with offsets
void vectorWithOffset(double* original, double* offset, double* result) {
    for (int i = 0; i < 3; i++) result[i] = original[i] + offset[i];
}

// sphere intersection
void checkIntersecionSphere(double* max_distance, double* startPoint, double* direction, IntersectionParameter** ip, int aa_num) {
    for (int i = 0; i < num_spheres; i++) {
        double sphere_center_with_offset[3];
        vectorWithOffset(spheres[i].position, anti_aliasing_filter[aa_num], sphere_center_with_offset);
        double b = 0;
        double c = -1 * spheres[i].radius * spheres[i].radius;
        for (int j = 0; j < 3; j++) {
            b += direction[j] * (startPoint[j]- sphere_center_with_offset[j]);
            c += (startPoint[j] - sphere_center_with_offset[j]) * (startPoint[j] - sphere_center_with_offset[j]);
        }
        b *= 2;
        double sqrtPart = b * b - 4 * c;
        if (sqrtPart < 0) continue;
        sqrtPart = sqrt(sqrtPart);
        double t0 = ( - b + sqrtPart) / 2;
        double t1 = ( - b - sqrtPart) / 2;
        double t = 0;  // t is distance
        if (t0 < 0 && t1 < 0) continue;
        if (t0 > 0 && t1 > 0) t = fmin(t0, t1);
        else t = fmax(t0, t1);
        if (t < ERROR_CONSTRAINT) continue;
        if (t >= *max_distance) continue;
        *max_distance = t;
        double p[3];
        double normal[3];
        vectorEquation(startPoint, direction, p, t);
        vectorFromTwoPoints(sphere_center_with_offset, p, normal);
        normalize(normal);
        if (ip != NULL) delete (*ip);
        // store intersection information into IntersectionParameter struct
        (*ip) = new IntersectionParameter();
        copyTriplet((*ip) -> normal, normal);
        copyTriplet((*ip) -> position, p);
        normalize(direction);
        copyTriplet((*ip) -> direction, direction);
        copyTriplet((*ip) -> color_diffuse, spheres[i].color_diffuse);
        copyTriplet((*ip) -> color_specular, spheres[i].color_specular);
        (*ip) -> shininess = spheres[i].shininess;
    }
}

// interpolate normal, interpolate material property values
void computeMaterialParameter(IntersectionParameter* ip, Vertex* source, double a, double b, double c) {
    for (int i = 0; i < 3; i++) {
        ip -> normal[i] = source[0].normal[i] * a + source[1].normal[i] * b + source[2].normal[i] * c;
        ip -> color_diffuse[i] = source[0].color_diffuse[i] * a + source[1].color_diffuse[i] * b + source[2].color_diffuse[i] * c;
        ip -> color_specular[i] = source[0].color_specular[i] * a + source[1].color_specular[i] * b + source[2].color_specular[i] * c;
    }
}

// test whether a point is inside the triangle
bool sameSideTest(double* a, double* b, double* c, double* p) {
    double ab[3], ap[3], ac[3], ab_ap[3], ab_ac[3];
    vectorFromTwoPoints(a, b, ab);
    vectorFromTwoPoints(a, p, ap);
    vectorFromTwoPoints(a, c, ac);
    vectorCrossProduct(ab, ap, ab_ap);
    vectorCrossProduct(ab, ac, ab_ac);
    if (dotProduct(ab_ap, ab_ac) >= 0) return true;
    return false;
}

// Find equation of the plane which the triangle lies on
// Find intersection of the ray and the plane
// Determine whether the ray-plane intersection point is inside or outside the triangle
void checkIntersecionTriangle(double* max_distance, double* startPoint, double* direction, IntersectionParameter** ip, int aa_num) {
    for (int i = 0; i < num_triangles; i++) {
        double triangle_vertex_with_offset[3][3];
        vectorWithOffset(triangles[i].v[0].position, anti_aliasing_filter[aa_num], triangle_vertex_with_offset[0]);
        vectorWithOffset(triangles[i].v[1].position, anti_aliasing_filter[aa_num], triangle_vertex_with_offset[1]);
        vectorWithOffset(triangles[i].v[2].position, anti_aliasing_filter[aa_num], triangle_vertex_with_offset[2]);

        double AB[3], AC[3], AP[3];
        vectorFromTwoPoints(triangle_vertex_with_offset[0], triangle_vertex_with_offset[1], AB);
        vectorFromTwoPoints(triangle_vertex_with_offset[0], triangle_vertex_with_offset[2], AC);
        double planeNormal[3];
        vectorCrossProduct(AB, AC, planeNormal);
        normalize(planeNormal);
        double* vertex0 = triangle_vertex_with_offset[0];
        // a⋅x + b⋅y + c⋅z + d = 0
        double a = planeNormal[0];
        double b = planeNormal[1];
        double c = planeNormal[2];
        double d = -(a * vertex0[0] + b * vertex0[1] + c * vertex0[2]);
        // plug in ray equations for x, y, z
        // x = x0 + xd t, y = y0 + yd t, z = z0 + zd t
        // a(x0 + xd t) + b(y0 + yd t) + c(z0 + zd t) + d = 0
        // t(a * xd + b * yd + c * zd) + a * x0 + b * y0 + c * z0 + d = 0
        // t = -(a * x0 + b * y0 + c * z0 + d) / (a * xd + b * yd + c * zd)
        double denominator = a * direction[0] + b * direction[1] + c * direction[2];
        if (denominator <= 0.0001 && denominator >= -0.0001) continue;
        double numerator = -(a * startPoint[0] + b * startPoint[1] + c * startPoint[2] + d);
        double t = numerator / denominator;
        if (t < ERROR_CONSTRAINT) continue;
        double P[3];
        vectorEquation(startPoint, direction, P, t);
    
        if(!sameSideTest(triangle_vertex_with_offset[1], triangle_vertex_with_offset[2], triangle_vertex_with_offset[0], P) ||
           !sameSideTest(triangle_vertex_with_offset[0], triangle_vertex_with_offset[2], triangle_vertex_with_offset[1], P) ||
           !sameSideTest(triangle_vertex_with_offset[0], triangle_vertex_with_offset[1], triangle_vertex_with_offset[2], P))
            continue;
        
        // at this point we know the intersecion lies inside the triangle
        if (t >= *max_distance) continue;
        *max_distance = t;
        if (ip != NULL) delete (*ip);
        (*ip) = new IntersectionParameter();
        copyTriplet((*ip) -> position, P);
        copyTriplet((*ip) -> direction, direction);
        double totalArea = triangleArea(triangle_vertex_with_offset[0], triangle_vertex_with_offset[1], triangle_vertex_with_offset[2]);
        double coeffieicent_a = triangleArea(P, triangle_vertex_with_offset[1], triangle_vertex_with_offset[0]) / totalArea;
        double coeffieicent_b = triangleArea(P, triangle_vertex_with_offset[1], triangle_vertex_with_offset[2]) / totalArea;
        double coeffieicent_c = triangleArea(P, triangle_vertex_with_offset[0], triangle_vertex_with_offset[2]) / totalArea;
        computeMaterialParameter(*ip, triangles[i].v, coeffieicent_a, coeffieicent_b, coeffieicent_c);
        (*ip) -> shininess = triangles[i].v[0].shininess * coeffieicent_a + triangles[i].v[1].shininess * coeffieicent_b + triangles[i].v[2].shininess * coeffieicent_c;
    }
}

IntersectionParameter* checkIntersecion(double* startPoint, double* direction, int aa_num) {
    double max_distance = DBL_MAX;
    IntersectionParameter* ip = NULL;
    checkIntersecionSphere(&max_distance, startPoint, direction, &ip, aa_num);
    checkIntersecionTriangle(&max_distance, startPoint, direction, &ip, aa_num);
    return ip;
}

void shading(IntersectionParameter* ip, double* color, int aa_num) {
    if (ip == NULL) return;
    for (int i = 0; i < 3; i++) color[i] = ambient_light[i] * 255;
    for (int i = 0; i < num_lights; i++) {
        double direction_light[3];
        vectorFromTwoPoints(ip->position, lights[i].position, direction_light);
        normalize(direction_light);
        IntersectionParameter* ip_light = checkIntersecion(ip->position, direction_light, aa_num);
        if (ip_light != NULL) {
            double distanceToLight = distanceSquare(ip->position, lights[i].position);
            double distanceToIntersection = distanceSquare(ip->position, ip_light->position);
            // this intersection lies in shadow
            if (distanceToIntersection <= distanceToLight) {
                delete ip_light;
                continue;
             }
        }
        // I = lightColor * (kd * (L dot N) + ks * (R dot V) ^ sh) (for each color channel separately; note that if L dot N < 0, you should clamp L dot N to zero; same for R dot V)
        normalize(direction_light);
        normalize(ip->normal);
        double LN = dotProduct(direction_light, ip->normal);
        double R[3];
        for (int j = 0; j < 3; j++) {
            R[j] = 2 * LN * ip->normal[j] - direction_light[j];
        }
        normalize(R);
        double V[3];
        for (int j = 0; j < 3; j++) V[j] = ip->direction[j] * -1;
        double RV = dotProduct(R, V);
        normalize(V);
        if(LN < 0) LN = 0;
        if (RV < 0) RV = 0;
        for (int j = 0; j < 3; j++) {
            color[j] += 255.0 * (lights[i].color[j] * (LN * ip->color_diffuse[j] + ip->color_specular[j] * pow(RV, ip->shininess)));
        }
    }
}

void raytracer(double* startPoint, double* direction, double* color, int aa_num) {
    IntersectionParameter* intersectionParameter = checkIntersecion(startPoint, direction, aa_num);
    if (intersectionParameter != NULL) {
        shading(intersectionParameter, color, aa_num);
        clamp(color, 0, 255.0);

    }
//    else {
//        initializeTriplet(color, BACKGROUND_COLOR_RGB, BACKGROUND_COLOR_RGB, BACKGROUND_COLOR_RGB);
//    }
    delete intersectionParameter;
}

//MODIFY THIS FUNCTION
void draw_scene(double* c_pos) {
  double direction[3];
  double color[3];
  for(unsigned int x = 0; x < WIDTH; x++) {
    glPointSize(2.0);  
    glBegin(GL_POINTS);
    for(unsigned int y = 0; y < HEIGHT; y++) {
        initializeTriplet(direction, start_x + x * increment_x, start_y + y * increment_y, -1);
        normalize(direction);
        double aa_color[3];
        initializeTriplet(aa_color, 0, 0, 0);
        for (int i = 0; i < anti_aliasing_size; i++) { // anti-aliasing
            initializeTriplet(color, BACKGROUND_COLOR_RGB, BACKGROUND_COLOR_RGB, BACKGROUND_COLOR_RGB);
            raytracer(c_pos, direction, color, i);
            for (int k = 0; k < 3; k++) aa_color[k] += color[k] * weight;
        }
        clamp(aa_color, 0, 255);
        plot_pixel(x, y, aa_color[0], aa_color[1], aa_color[2]);
    }
    glEnd();
    glFlush();
  }
  printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
  glColor3f(((double)r)/256.f,((double)g)/256.f,((double)b)/256.f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
  buffer[HEIGHT-y-1][x][0]=r;
  buffer[HEIGHT-y-1][x][1]=g;
  buffer[HEIGHT-y-1][x][2]=b;
}

void plot_pixel(int x,int y,unsigned char r,unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
      plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  Pic *in = NULL;

  in = pic_alloc(640, 480, 3, NULL);
  printf("Saving JPEG file: %s\n", filename);

  memcpy(in->pix,buffer,3*WIDTH*HEIGHT);
  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);      

}

void parse_check(char *expected,char *found)
{
  if(strcasecmp(expected,found))
    {
      char error[100];
      printf("Expected '%s ' found '%s '\n",expected,found);
      printf("Parse error, abnormal abortion\n");
      exit(0);
    }

}

void parse_doubles(FILE*file, char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE*file,double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE*file,double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE *file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  int i;
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i",&number_of_objects);

  printf("number of objects: %i\n",number_of_objects);
  char str[200];

  parse_doubles(file,"amb:",ambient_light);

  for(i=0;i < number_of_objects;i++)
    {
      fscanf(file,"%s\n",type);
      printf("%s\n",type);
      if(strcasecmp(type,"triangle")==0)
	{

	  printf("found triangle\n");
	  int j;

	  for(j=0;j < 3;j++)
	    {
	      parse_doubles(file,"pos:",t.v[j].position);
	      parse_doubles(file,"nor:",t.v[j].normal);
	      parse_doubles(file,"dif:",t.v[j].color_diffuse);
	      parse_doubles(file,"spe:",t.v[j].color_specular);
	      parse_shi(file,&t.v[j].shininess);
	    }

	  if(num_triangles == MAX_TRIANGLES)
	    {
	      printf("too many triangles, you should increase MAX_TRIANGLES!\n");
	      exit(0);
	    }
	  triangles[num_triangles++] = t;
	}
      else if(strcasecmp(type,"sphere")==0)
	{
	  printf("found sphere\n");

	  parse_doubles(file,"pos:",s.position);
	  parse_rad(file,&s.radius);
	  parse_doubles(file,"dif:",s.color_diffuse);
	  parse_doubles(file,"spe:",s.color_specular);
	  parse_shi(file,&s.shininess);

	  if(num_spheres == MAX_SPHERES)
	    {
	      printf("too many spheres, you should increase MAX_SPHERES!\n");
	      exit(0);
	    }
	  spheres[num_spheres++] = s;
	}
      else if(strcasecmp(type,"light")==0)
	{
	  printf("found light\n");
	  parse_doubles(file,"pos:",l.position);
	  parse_doubles(file,"col:",l.color);

	  if(num_lights == MAX_LIGHTS)
	    {
	      printf("too many lights, you should increase MAX_LIGHTS!\n");
	      exit(0);
	    }
	  lights[num_lights++] = l;
	}
      else
	{
	  printf("unknown type in scene description:\n%s\n",type);
	  exit(0);
	}
    }
  return 0;
}

void display() {}

void init() {
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
  initializeTriplet(camera_pos, 0, 0, 0);
  aspect_ratio = WIDTH * 1.0 / (1.0 * HEIGHT);
  start_y = -tan((fov * PI / 180.0) / 2);
  start_x = start_y * aspect_ratio;
  increment_x = -(start_x * 2) / WIDTH;
  increment_y = -(start_y * 2) / HEIGHT;
}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once) {
      initializeTriplet(camera_pos, camera_pos[0], camera_pos[1], camera_pos[2]);
      draw_scene(camera_pos);
      if(mode == MODE_JPEG)
	  save_jpg();
  }
  once=1;
}

int main (int argc, char ** argv)
{
  if (argc<2 || argc > 3)
  {  
    printf ("usage: %s <scenefile> [jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
    {
      mode = MODE_JPEG;
      filename = argv[2];
    }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}
