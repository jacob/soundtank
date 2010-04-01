INCLUDES = `xml2-config --cflags`

AM_CFLAGS = $(SOUNDTANK_CFLAGS) 

bin_PROGRAMS = soundtank

soundtank_SOURCES = soundtank.c soundtank_startup.c soundtank_types.c buffer.c channel.c data_port.c control.c engine.c engine_alsa.c engine_jack.c process.c util/linked_list.c util/generic_array.c util/string_to_argv.c util/namespace.c rtobject.c rtobject_instance.c xml_file.c event_map/event_map.c event_map/map_test.c event_map/map_action.c event_map/scale.c event_map/rt_route_ev.c event_map/tests/test_callbacks.c event_map/tests/test_print_event_type.c event_map/tests/test_true.c event_map/tests/test_event_type.c event_map/tests/test_event_param_in_range.c event_map/actions/action_callbacks.c event_map/actions/jump.c event_map/actions/set_control.c event_map/actions/note_to_pitch.c event_map/actions/set_note_flag.c event_map/auto_map.c imp_objects/imp_objects.c imp_objects/local_in_and_out.c imp_objects/signal_path.c imp_objects/alsa_extern.c imp_objects/jack_extern.c imp_objects/test_source.c imp_objects/ladspa_plugin.c imp_objects/ladspa_load.c imp_objects/ladspa_listplugins.c imp_objects/ladspa_search.c imp_objects/ladspa_default.c imp_objects/channel_ops.c commands/commands.c commands/engine_commands.c commands/stat.c commands/ls.c commands/cd.c commands/cr.c commands/sv.c commands/mv.c commands/at.c commands/set.c commands/map.c commands/debug.c

soundtank_LDFLAGS = `xml2-config --libs`

soundtank_LDADD = -lasound -ljack -lpopt -lreadline -lncurses 
