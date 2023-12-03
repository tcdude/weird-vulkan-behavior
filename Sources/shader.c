#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/io/filereader.h>
#include <kinc/math/matrix.h>
#include <kinc/math/random.h>
#include <kinc/math/vector.h>
#include <kinc/system.h>

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "model.h"

#define INST_COUNT 2000

static kinc_g4_shader_t vertex_shader;
static kinc_g4_shader_t fragment_shader;
static kinc_g4_pipeline_t pipeline;
static kinc_g4_vertex_buffer_t vertices;
static float transform[INST_COUNT * 6];
static kinc_g4_index_buffer_t indices;
static kinc_g4_constant_location_t mvp_loc;
static kinc_g4_constant_location_t v_loc;
static kinc_g4_constant_location_t lightPos_loc;
static kinc_g4_constant_location_t off_loc;
static kinc_g4_constant_location_t rot_loc;
static kinc_matrix4x4_t mvp_mat;
static kinc_matrix4x4_t v_mat;
static kinc_vector3_t light_pos;

#define HEAP_SIZE 1024 * 1024
static uint8_t *heap = NULL;
static size_t heap_top = 0;

static void *allocate(size_t size) {
  size_t old_top = heap_top;
  heap_top += size;
  assert(heap_top <= HEAP_SIZE);
  return &heap[old_top];
}

static void update(void *data) {
  kinc_g4_begin(0);
  kinc_g4_clear(KINC_G4_CLEAR_COLOR | KINC_G4_CLEAR_DEPTH, 0xff111211, 1, 0);
  double now = kinc_time();
  float st = sinf(now / 3);
  float ct = cosf(now / 3);
  kinc_vector3_t eye = (kinc_vector3_t){ct * 55, 0, st * 55};
  st = sinf(-now);
  ct = cosf(-now);
  light_pos = (kinc_vector3_t){ct * 22, st * 22, 0};
  kinc_matrix4x4_t proj =
      perspective_projection(1.57, 9.0f / 16.0f, 0.1f, 100.0f);
  v_mat = look_at(eye, (kinc_vector3_t){0, 0, 0}, (kinc_vector3_t){0, 1, 0});
  mvp_mat = kinc_matrix4x4_multiply(&proj, &v_mat);

  kinc_g4_set_pipeline(&pipeline);
  kinc_g4_set_vertex_buffer(&vertices);
  kinc_g4_set_index_buffer(&indices);
  kinc_g4_set_matrix4(mvp_loc, &mvp_mat);
  kinc_g4_set_matrix4(v_loc, &v_mat);
  kinc_g4_set_floats(lightPos_loc, (float *)&light_pos, 3);

  for (int i = 0; i < INST_COUNT; ++i) {
    kinc_g4_set_floats(off_loc, &transform[i * 6], 3);
    kinc_g4_set_floats(rot_loc, &transform[i * 6 + 3], 3);
    kinc_g4_draw_indexed_vertices();
  }

  kinc_g4_end(0);
  kinc_g4_swap_buffers();
}

static void load_shader(const char *filename, kinc_g4_shader_t *shader,
                        kinc_g4_shader_type_t shader_type) {
  kinc_file_reader_t file;
  kinc_file_reader_open(&file, filename, KINC_FILE_TYPE_ASSET);
  size_t data_size = kinc_file_reader_size(&file);
  uint8_t *data = allocate(data_size);
  kinc_file_reader_read(&file, data, data_size);
  kinc_file_reader_close(&file);
  kinc_g4_shader_init(shader, data, data_size, shader_type);
}

int kickstart(int argc, char **argv) {
  kinc_init("Shader", 450, 800, NULL, NULL);
  kinc_set_update_callback(update, NULL);

  heap = (uint8_t *)malloc(HEAP_SIZE);
  assert(heap != NULL);

  load_shader("shader.vert", &vertex_shader, KINC_G4_SHADER_TYPE_VERTEX);
  load_shader("shader.frag", &fragment_shader, KINC_G4_SHADER_TYPE_FRAGMENT);

  kinc_g4_vertex_structure_t structure;
  kinc_g4_vertex_structure_init(&structure);
  kinc_g4_vertex_structure_add(&structure, "pos", KINC_G4_VERTEX_DATA_F32_3X);
  kinc_g4_vertex_structure_add(&structure, "uv", KINC_G4_VERTEX_DATA_F32_2X);
  kinc_g4_vertex_structure_add(&structure, "normal",
                               KINC_G4_VERTEX_DATA_F32_3X);
  kinc_g4_pipeline_init(&pipeline);
  pipeline.vertex_shader = &vertex_shader;
  pipeline.fragment_shader = &fragment_shader;
  pipeline.input_layout[0] = &structure;
  pipeline.input_layout[1] = NULL;
  pipeline.depth_write = true;
  pipeline.depth_mode = KINC_G4_COMPARE_LESS;
  pipeline.cull_mode = KINC_G4_CULL_CLOCKWISE;
  pipeline.blend_source = KINC_G4_BLEND_SOURCE_ALPHA;
  pipeline.blend_destination = KINC_G4_BLEND_INV_SOURCE_ALPHA;
  pipeline.alpha_blend_source = KINC_G4_BLEND_SOURCE_ALPHA;
  pipeline.alpha_blend_destination = KINC_G4_BLEND_INV_SOURCE_ALPHA;
  kinc_g4_pipeline_compile(&pipeline);

  kinc_g4_index_buffer_init(&indices, INDEX_COUNT,
                            KINC_G4_INDEX_BUFFER_FORMAT_16BIT,
                            KINC_G4_USAGE_STATIC);
  kinc_g4_vertex_buffer_init(&vertices, VERTEX_COUNT, &structure,
                             KINC_G4_USAGE_STATIC, 0);
  {
    float *v = kinc_g4_vertex_buffer_lock_all(&vertices);
    uint16_t *i = (uint16_t *)kinc_g4_index_buffer_lock_all(&indices);
    memcpy(v, vertex_buffer, VERTEX_COUNT * sizeof(float) * 8);
    memcpy(i, index_buffer, INDEX_COUNT * sizeof(uint16_t));
    kinc_g4_index_buffer_unlock_all(&indices);
    kinc_g4_vertex_buffer_unlock_all(&vertices);
  }

  kinc_random_init(1234);

  {
    float *v = transform;
    int i = 0;
    for (int inst = 0; inst < INST_COUNT; ++inst) {

      v[i++] = (float)kinc_random_get_in(-400, 400) / 10.0f;
      v[i++] = (float)kinc_random_get_in(-400, 400) / 10.0f;
      v[i++] = (float)kinc_random_get_in(-400, 400) / 10.0f;
      v[i++] = 0;
      v[i++] = 0;
      v[i++] = 0;
    }
  }
  mvp_loc = kinc_g4_pipeline_get_constant_location(&pipeline, "mvp");
  v_loc = kinc_g4_pipeline_get_constant_location(&pipeline, "V");
  lightPos_loc = kinc_g4_pipeline_get_constant_location(&pipeline, "lightPos");
  off_loc = kinc_g4_pipeline_get_constant_location(&pipeline, "off");
  rot_loc = kinc_g4_pipeline_get_constant_location(&pipeline, "rot");
  light_pos = (kinc_vector3_t){10, 10, 10};

  kinc_start();

  return 0;
}
