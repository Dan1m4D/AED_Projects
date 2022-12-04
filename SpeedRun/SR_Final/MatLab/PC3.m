clear all;
nVector = [10:1:50 55]
resultVector = [3.500e-06
                6.054e-06
                6.806e-06
                1.001e-05
                1.547e-05
                2.358e-05
                3.843e-05
                6.640e-05
                1.074e-04
                1.971e-04
                2.818e-04
                4.744e-04
                7.845e-04
                1.310e-03
                2.171e-03
                3.539e-03
                5.960e-03
                9.959e-03
                1.625e-02
                2.542e-02
                4.232e-02
                7.050e-02
                1.159e-01
                1.932e-01
                3.226e-01
                5.358e-01
                8.908e-01
                1.396e+00
                1.426e+00
                2.382e+00
                4.222e+00
                6.864e+00
                1.141e+01
                1.883e+01
                3.093e+01
                5.253e+01
                8.489e+01
                1.410e+02
                2.558e+02
                3.959e+02
                6.811e+02
                7.359e+03]';

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