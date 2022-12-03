clear;


%% Declaração dos valores
nVector = [10:1:50 55:5:100 110:10:200 220:20:800];
resultVector = [1.133e-06 2.428e-06 8.700e-07 1.075e-06 ...];

%% Plot dos valores
figure(1);
hold on;
plot(nVector, resultVector, "+r");
plot(nVector, resultVector, "b");
legend("Solução recursíva final");
axis([0 800 0 10e-06]);
xlabel("n");
ylabel("CPU Time (segundos)");
grid on;
hold off;