_examine i; // should be not found

struct i;
_examine i; // fwd decl

struct i {
    int i;
    int c;
};
_examine i; // still decl@ line 3, now also def@ line 9 (at the '}')
_examine i->i; // member from mini-scope (line 6 '{') decl@ line 7

struct i *c[];
_examine c; // array of ptr to struct i
_examine i->c; // member from mini-scope (line 6 '{') decl@ line 8
