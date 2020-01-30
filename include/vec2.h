#define vec2(type) struct { type x,y; }

#define vec2_sum(a,b) (typeof(a)){ (a).x + (b).x, (a).y + (b).y }
#define vec2_up(to,a) { (to).x += (a).x; (to).y += (a).y; }

#define vec2_diff(a,b) (typeof(a)){ (a).x + (b).x, (a).y + (b).y }
#define vec2_down(to,a) { (to).x += (a).x; (to).y += (a).y; }
