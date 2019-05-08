clear
clc
load('fig_mmg.mat')

figure(1)
clf

subplot(3,1,1)
plot(tvec(1:4000,:),xvec(4001:8000,1));
hold on
plot(tvec(1:4000,:),xdesvec(4001:8000,1));
legend theta theta_{des}
xlabel('Time(s)')
ylabel('theta(rad)')

subplot(3,1,2)
plot(tvec(1:4000,:),ai(8001:2:16000,1));
xlabel('Time(s)')
ylabel('mmg(V)')
ylim([-1 1])
legend mmg1

subplot(3,1,3)
plot(tvec(1:4000,:),ai(8001:2:16000,2));
xlabel('Time(s)')
ylabel('mmg(V)')
ylim([-1 1])
legend mmg2

saveas(gcf,'./fig_mmg.epsc')