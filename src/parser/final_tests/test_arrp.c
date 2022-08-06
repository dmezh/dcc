int printf();

int main() {
        char helloworld[15];
        helloworld[0] = 'h';
        helloworld[1] = 'e';
        helloworld[2] = 'l';
        helloworld[3] = 'l';
        helloworld[4] = 'o';
        helloworld[5] = ' ';
        helloworld[6] = 'w';
        helloworld[7] = 'o';
        helloworld[8] = 'r';
        helloworld[9] = 'l';
        helloworld[10] = 'd';
        helloworld[11] = '!';
        helloworld[12] = '\n';

        printf(helloworld);
        return 0;
}
