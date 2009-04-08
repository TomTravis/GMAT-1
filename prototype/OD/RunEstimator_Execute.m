function RunEstimator_Exectute(Estimator,Propagator)

global ObjectStore

%==========================================================================
%==========================================================================
%---- Perform the last bits of initialization
%==========================================================================
%==========================================================================
Estimator  = RunEstimator_PreparetoSolve(Estimator);

%==========================================================================
%==========================================================================
%---- Run the State Machine
%==========================================================================
%==========================================================================

load RangeObs.mat Yo;
ObjectStoreClone = ObjectStore;

diff = 1;
tol  = 1e-5;

disp(' Iter    norm(Yo - Yc)      norm(dx)');
disp('------------------------------------')
count = 0;
while diff > tol

    ObjectStore = ObjectStoreClone;
    Propagator = RunEstimator_PreparetoPropagate(Propagator,Estimator);
    EpochVec   = Propagator.Epoch + [0:300:86400]/86400;
    count      = count + 1;

    for i = 1:size(EpochVec,2)

        %  Step to next measurement epoch
        NewEpoch   = EpochVec(i);
        Propagator = Propagator_StepToEpoch(Propagator,NewEpoch);

        %  Call the measurement model
        [g,dgdrv]   = GSMeasurement_Evaluate(ObjectStore.Objects{3});
        STM         = ObjectStore.Objects{1}.STM;
        Y(i,1)      = g;
        dgdvv       = zeros(1,3);
        Htilde      = [dgdrv dgdvv];
        H(i,:)      = Htilde*STM;

    end

    resid = Yo - Y;
    dx = inv(H'*H)*H'*resid;
    diff = norm(dx);

    Estimator.ESV = Estimator.ESV + dx;

    %  This is a kludge to update the spacecraft state.  Need to add a
    %  Estimator_UpdateState method or something.
    ObjectStoreClone.Objects{1}.X = Estimator.ESV(1,1);
    ObjectStoreClone.Objects{1}.Y = Estimator.ESV(2,1);
    ObjectStoreClone.Objects{1}.Z = Estimator.ESV(3,1);
    ObjectStoreClone.Objects{1}.VX = Estimator.ESV(4,1);
    ObjectStoreClone.Objects{1}.VY = Estimator.ESV(5,1);
    ObjectStoreClone.Objects{1}.VZ = Estimator.ESV(6,1);
 
    disp([ '  ' num2str(count)  '         ' num2str(norm(resid),4)  '        '  num2str(norm(dx),4) ] );

end

disp(' ')
disp('Batch Least Squares Has Converged!!')
