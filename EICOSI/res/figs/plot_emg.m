clear
clc
load('fig_emg.mat')

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
ylabel('emg(V)')
ylim([-0.25 0.25])
legend emg1

subplot(3,1,3)
plot(tvec(1:4000,:),ai(8001:2:16000,2));
xlabel('Time(s)')
ylabel('emg(V)')
ylim([-0.25 0.25])
legend emg2

saveas(gcf,'./fig_emg.png')