%% Plot
figure(1)
clf

subplot(3,2,1)
plot(tvec(:,:),xvec(:,1));
hold on
plot(tvec(:,:),xdesvec(:,1));
grid on
legend theta thetades

% subplot(3,2,2)
% %plot(tvec(2:end,:),diff(xvec(:,1))/(tvec(2)-tvec(1)));
% plot(tvec(:,:),xvec(:,2));
% grid on
% legend dtheta

subplot(3,2,3)
plot(ai(:,1));  
hold on
plot(xvec(:,5));
% ylim([-0.1 0.25])
grid on
legend ai1 m1

subplot(3,2,4)
plot(ai(:,2));  
hold on
plot(xvec(:,6));
% ylim([-0.1 0.25])
grid on
legend ai2 m2

subplot(3,2,5)
plot(tvec(:,:),uvec(:,:));
grid on
legend u

if exist('mode','var')
    subplot(3,2,6)
    plot(tvec(:,:),mode(:,:));
    grid on
    legend mode
else
    disp('no mode var exists')
end