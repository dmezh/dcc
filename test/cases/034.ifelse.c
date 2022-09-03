//!dtest description if/else.
//!dtest expect returncode 1

int main() {
    int i = 100;

    if (i < 1000) {
        i++;
    }

    if (i == 666)
        i += 100;
    else
        if (i > 200)
            i = 0;
        else
            i -= 100;

    return i;
}
