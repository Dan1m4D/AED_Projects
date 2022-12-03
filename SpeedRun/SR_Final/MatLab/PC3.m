clear all;
nVector = [10:1:50]
resultVector = []';

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