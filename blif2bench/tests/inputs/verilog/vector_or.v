module top(a, y);
input [1:0] a;
output y;
or g1 (y, a[1], a[0]);
endmodule
