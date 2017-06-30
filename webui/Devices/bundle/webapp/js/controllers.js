'use strict';

var devicesControllers = angular.module('devicesControllers', []);

devicesControllers.service('DeviceService', ['$http',
  function($http) {
	//var inputdata = $.param({action : "msgPa", bundle : "io.bundle.devices"});
    this.post = function(inputAction, inputSymbolicName, inputId) {
    	var inputdata = $.param({action : inputAction, bundle : inputSymbolicName, input : inputId});
    	var request =     		  
    	{ 
		        method: "POST", 
		        url: "/macchina/bundles/actions.json",
		        headers: {
		          "Content-Type": "application/x-www-form-urlencoded"
		        },
		        data: inputdata
    	};
    	$http(request)
    		.success(function() {})
    		.error(function() {});
    };

    this.paMsg = function(symbolicName, inputId) {
      this.post('paMsg', symbolicName, inputId);
    };
    this.simplepaMsg = function() {
        //alert("inside the simplepaMsg");
        this.post('paMsg', 'io.macchina.instant', '02');
     };
  }
]);

devicesControllers.controller('DevicesCtrl', ['$scope', '$http', '$interval','DeviceService',
  function ($scope, $http, $interval, DeviceService) {
	$scope.states = {};	//
	$scope.states.activeItem = 'PA.instantMessage.1'; //
	
    $scope.devices = [];
    $scope.orderBy = "name";
    
    $scope.sendPaMessage = function(msgId) {
    	//alert(msgId);
    	//var deviceRefs = serviceRegistry.find('io.macchina.device != ""');
    	//var deviceRef = deviceRefs[0];
    	//var device = deviceRef.instance();
    	DeviceService.paMsg('io.macchina.sched', msgId);
        DeviceService.simplepaMsg();
      }
    
    $scope.setOrderBy = function(col) {
      $scope.orderBy = col;
    }
    $http.get('/macchina/devices/devices.jss').success(function(data) {
      $scope.devices = data;
    });
    /** frequent update of the status TODO devices.jss pass to DeviceImpl.h
    $interval(function() {
    console.log("frequent update of the status");
      $http.get('/macchina/devices/devices.jss').success(function(data) {
        $scope.devices = data;
      })
    }, 1000);
    **/
  }]);

devicesControllers.controller('SessionCtrl', ['$scope', '$http',
  function($scope, $http) {
    $http.get('/macchina/session.json').success(function(data) {
      $scope.session = data;
      if (!data.authenticated)
      {
        window.location = "/";
      }
    });
  }]);
