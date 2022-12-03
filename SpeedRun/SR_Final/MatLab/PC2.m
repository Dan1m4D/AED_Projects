clear all;
nVector = [10:1:50]
resultVector = [4.600e-06
                8.898e-06
                1.059e-05
                1.612e-05
                2.745e-05
                4.181e-05
                6.837e-05
                1.127e-04
                1.885e-04
                3.099e-04
                5.201e-04
                3.403e-04
                5.569e-04
                9.212e-04
                1.281e-03
                1.724e-03
                2.927e-03
                4.644e-03
                7.727e-03
                1.259e-02
                2.150e-02
                3.635e-02
                6.033e-02
                1.006e-01
                1.697e-01
                2.801e-01
                4.706e-01
                7.762e-01
                1.201e+00
                2.053e+00
                3.356e+00
                5.577e+00
                9.308e+00
                1.569e+01
                2.730e+01
                4.596e+01
                7.587e+01
                1.288e+02
                2.118e+02
                3.450e+02
                5.707e+02]';

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