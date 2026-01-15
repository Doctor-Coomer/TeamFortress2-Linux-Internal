
void (*scene_end_original)(void*, void*);

void scene_end_hook(void* me, void* b8) {
  scene_end_original(me, b8);
  //chams();
}
