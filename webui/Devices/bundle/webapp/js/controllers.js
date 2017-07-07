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
    		.success(	function() {	alert("Pa Message Sent successfully");})
    		.error(		function() {	alert("Oops.. something wrong");});
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
	$scope.states.activeItem = '1'; //
	
    $scope.devices = [];
    $scope.orderBy = "name";
    
    $scope.sendPaMessage = function(msgId) {
    	//by sam 20170705
        $http.get('/macchina/devices/devices.jss').success(function(data) {
            $scope.devices = data;
          });
        
    	DeviceService.paMsg('io.macchina.sched', msgId);
        //tmp debugging in case there is no DeviceService.simplepaMsg();
    	
      }
    
    $scope.setOrderBy = function(col) {
      $scope.orderBy = col;
    }
    $http.get('/macchina/devices/devices.jss').success(function(data) {
      $scope.devices = data;
    });
    
    $interval(function() {
    console.log("frequent update of the status");
      $http.get('/macchina/devices/devices.jss').success(function(data) {
        $scope.devices = data;
      })
    }, 1000);
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
