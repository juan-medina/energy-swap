// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0;
uniform float screen_width;
uniform float screen_height;
uniform float time;

// Curvature constants
const bool curvature = false;
const float barrel_power = 1.025;

// Color bleeding constants
uniform bool color_bleed;
const float color_bleeding = 1.2;
const float bleeding_range_x = 1.0;
const float bleeding_range_y = 1.0;

// Scanline constants
uniform bool scan_lines;
const float lines_distance = 2.0;
const float scan_size = 1.0;
const float scanline_alpha = 0.85;
const float lines_velocity = 20.0;

vec2 distort(vec2 p) {
    float theta = atan(p.y, p.x);
    float radius = pow(length(p), barrel_power);

    p.x = radius * cos(theta);
    p.y = radius * sin(theta);

    return 0.5 * (p + vec2(1.0, 1.0));
}

void get_color_bleeding(inout vec4 current_color, inout vec4 color_left) {
    current_color = current_color * vec4(color_bleeding, 0.5, 1.0 - color_bleeding, 1.0);
    color_left = color_left * vec4(1.0 - color_bleeding, 0.5, color_bleeding, 1.0);
}

void get_color_scanline(vec2 uv, inout vec4 c, float t) {
    float line_row = floor((uv.y * screen_height / scan_size) + mod(t * lines_velocity, lines_distance));
    float n = 1.0 - ceil((mod(line_row, lines_distance) / lines_distance));
    c = c - n * c * (1.0 - scanline_alpha);
    c.a = 1.0;
}

void main() {
    vec2 xy = fragTexCoord;

    if (curvature) {
        xy = fragTexCoord * 2.0;

        xy.x -= 1.0;
        xy.y -= 1.0;

        float d = length(xy);
        if (d < 1.5) {
            xy = distort(xy);
        } else {
            xy = fragTexCoord;
        }
    }

    vec4 current_color = texture2D(texture0, xy);

    if (color_bleed) {
        float pixel_size_x = 1.0 / screen_width * bleeding_range_x;
        float pixel_size_y = 1.0 / screen_height * bleeding_range_y;
        vec4 color_left = texture2D(texture0, xy - vec2(pixel_size_x, pixel_size_y));
        get_color_bleeding(current_color, color_left);
        current_color += color_left;
    }

    if (scan_lines) {
        get_color_scanline(xy, current_color, time);
    }

    gl_FragColor = current_color;
}
