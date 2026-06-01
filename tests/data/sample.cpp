int foo(double value) {
    int x = value; // implicit double -> int
    bool b = x;    // implicit int -> bool
    return b;
}
