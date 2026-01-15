#ifndef RENDER_CONTEXT_HPP
#define RENDER_CONTEXT_HPP

enum MaterialCullMode {
  MATERIAL_CULLMODE_CCW,
  MATERIAL_CULLMODE_CW
};

enum StencilComparisonMode {
  STENCILCOMPARISONFUNCTION_NEVER = 1,
  STENCILCOMPARISONFUNCTION_LESS = 2,
  STENCILCOMPARISONFUNCTION_EQUAL = 3,
  STENCILCOMPARISONFUNCTION_LESSEQUAL = 4,
  STENCILCOMPARISONFUNCTION_GREATER = 5,
  STENCILCOMPARISONFUNCTION_NOTEQUAL = 6,
  STENCILCOMPARISONFUNCTION_GREATEREQUAL = 7,
  STENCILCOMPARISONFUNCTION_ALWAYS = 8,
  STENCILCOMPARISONFUNCTION_FORCE_DWORD = 0x7fffffff
};

enum StencilOperation {
  STENCILOPERATION_KEEP = 1,
  STENCILOPERATION_ZERO = 2,
  STENCILOPERATION_REPLACE = 3,
  STENCILOPERATION_INCRSAT = 4,
  STENCILOPERATION_DECRSAT = 5,
  STENCILOPERATION_INVERT = 6,
  STENCILOPERATION_INCR = 7,
  STENCILOPERATION_DECR = 8,
  STENCILOPERATION_FORCE_DWORD = 0x7fffffff
};

class RenderContext {
public:

  //11
  void set_depth_range(float near, float far) {
    void** vtable = *(void***)this;

    void (*set_depth_range_fn)(void*, float, float) = (void (*)(void*, float, float))vtable[11];

    set_depth_range_fn(this, near, far);
  }

  
  void set_stencil_test_mask(int mask) {
    void** vtable = *(void***)this;

    void (*set_stencil_test_fn)(void*, int) = (void (*)(void*, int))vtable[123];

    set_stencil_test_fn(this, mask);    
  }

  void set_stencil_write_mask(int mask) {
    void** vtable = *(void***)this;

    void (*set_stencil_write_fn)(void*, int) = (void (*)(void*, int))vtable[124];

    set_stencil_write_fn(this, mask);    
  }
  
  void set_stencil_reference_count(int reference_count) {
    void** vtable = *(void***)this;

    void (*set_stencil_reference_count_fn)(void*, int) = (void (*)(void*, int))vtable[122];

    set_stencil_reference_count_fn(this, reference_count);    
  }

  void set_stencil_fail_mode(StencilOperation stencil_operation) {
    void** vtable = *(void***)this;

    void (*set_stencil_fail_mode_fn)(void*, StencilOperation) = (void (*)(void*, StencilOperation))vtable[118];

    set_stencil_fail_mode_fn(this, stencil_operation);    
  }
  
  void set_stencil_zfail_mode(StencilOperation stencil_operation) {
    void** vtable = *(void***)this;

    void (*set_stencil_zfail_mode_fn)(void*, StencilOperation) = (void (*)(void*, StencilOperation))vtable[119];

    set_stencil_zfail_mode_fn(this, stencil_operation);    
  }
  
  void set_stencil_pass_mode(StencilOperation stencil_operation) {
    void** vtable = *(void***)this;

    void (*set_stencil_pass_mode_fn)(void*, StencilOperation) = (void (*)(void*, StencilOperation))vtable[120];

    set_stencil_pass_mode_fn(this, stencil_operation);    
  }

  
  void set_stencil_compare_mode(StencilComparisonMode stencil_compare_mode) {
    void** vtable = *(void***)this;

    void (*set_stencil_compare_mode_fn)(void*, StencilComparisonMode) = (void (*)(void*, StencilComparisonMode))vtable[121];

    set_stencil_compare_mode_fn(this, stencil_compare_mode);    
  }
  
  void clear_buffers(bool clear_color, bool clear_depth, bool clear_stencil) {
    void** vtable = *(void***)this;

    void (*clear_buffers_fn)(void*, bool, bool, bool) = (void (*)(void*, bool, bool, bool))vtable[12];

    clear_buffers_fn(this, clear_color, clear_depth, clear_stencil);
  }
  
  
  void set_stencil_enable(bool value) {
    void** vtable = *(void***)this;

    void (*set_stencil_enable_fn)(void*, bool) = (void (*)(void*, bool))vtable[117];

    set_stencil_enable_fn(this, value);
  }
  
  void set_cull_mode(MaterialCullMode cull_mode) {
    void** vtable = *(void***)this;

    void (*set_cull_mode_fn)(void*, MaterialCullMode) = (void (*)(void*, MaterialCullMode))vtable[40];

    set_cull_mode_fn(this, cull_mode);
  }
  
};

#endif
