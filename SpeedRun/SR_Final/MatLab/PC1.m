clear all;
nVector = [10:1:50 55]
resultVector = [4.017e-06
                7.874e-06
                9.067e-06
                1.357e-05
                2.157e-05
                3.475e-05
                5.686e-05
                9.362e-05
                1.564e-04
                2.574e-04
                4.281e-04
                7.580e-04
                1.257e-03
                2.149e-03
                3.484e-03
                5.901e-03
                9.677e-03
                1.679e-02
                2.720e-02
                4.241e-02
                7.476e-02
                1.242e-01
                2.094e-01
                3.499e-01
                4.994e-01
                3.967e-01
                6.514e-01
                1.107e+00
                1.850e+00
                2.777e+00
                4.401e+00
                8.449e+00
                1.327e+01
                2.313e+01
                3.572e+01
                5.652e+01
                9.449e+01
                1.644e+02
                2.640e+02
                4.323e+02
                6.971e+02
                7.358e+03]';

figure(1);
hold on;
plot(nVector, resultVector, "+r");
plot(nVector, resultVector, "b");
legend("Solução recursíva inicial");
axis([0 60 0 10000]);
xlabel("n");
ylabel("CPU Time (segundos)");
grid on;
hold off;