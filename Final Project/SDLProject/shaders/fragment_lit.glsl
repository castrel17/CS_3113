uniform sampler2D diffuse;
uniform vec2 lightPosition;

varying vec2 texCoordVar;
varying vec2 varPosition;
uniform int spotlight_on;
uniform int attack_on;

float attenuate(float dist, float a, float b)
{
     return 1.0 / (1.0 + (a * dist) + (b * dist  * dist));
}

void main()
{
    // The brightness is directly based on the distance between the light source's
    // location and the pixel's location
    float brightness = attenuate(distance(lightPosition, varPosition), 3.0, 0.0);
    vec4 color = texture2D(diffuse, texCoordVar);

    if(spotlight_on == 0){ //1 is on, 0 is off
        gl_FragColor = vec4(color.rgb, color.a);
    }else{
        if(attack_on == 1){
            gl_FragColor = vec4(color.r * brightness, 0, 0, color.a);
        }else{
            gl_FragColor = vec4(color.rgb*brightness, color.a);
        }
    }
}
