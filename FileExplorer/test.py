break;
case 26://ctrl-z
{
    undo( editor_state.es );
    editor_state.cur_x = editor_state.es->cur_x;
    editor_state.cur_y = editor_state.es->cur_y;
}
break;
case 18://c