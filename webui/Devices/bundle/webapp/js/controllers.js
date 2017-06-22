'use strict';

var devicesControllers = angular.module('devicesControllers', []);

devicesControllers.service('DeviceService', ['$http',
  function($http) {
    this.post = function(action, symbolicName) {
      $http({ 
        method: "POST", 
        url: "/macchina/bundles/actions.json",
        headers: {
          "Content-Type": "application/x-www-form-urlencoded"
        },
        transformRequest: function(obj) {
          var str = [];
          for (var p in obj) {
            str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
          }
          return str.join("&");
        },
        data: {
            action: action,
            symbolicName: symbolicName
        }
      })
      .success(function(data) {
    	  
      })
      .error(function() {
    	  	
      });
    };

    this.paMsg = function(symbolicName) {
      this.post("paMsg", symbolicName);
    };
    this.simplepaMsg = function() {
        //alert("inside the simplepaMsg");
        this.post("paMsg", "io");
     };
  }
]);

devicesControllers.controller('DevicesCtrl', ['$scope', '$http', '$interval','DeviceService',
  function ($scope, $http, $interval, DeviceService) {
	$scope.states = {};	//
	$scope.states.activeItem = 'PA.instantMessage.1'; //
	
    $scope.devices = [];
    $scope.orderBy = "name";
    
    $scope.sendPaMessage = function() {
    	//alert(document.getElementById('text').value);
    	//window.alert("this.paMsg");
        //alert("before the DeviceService.paMsg");
    	//DeviceService.paMsg($scope.bundle.symbolicName);
        DeviceService.simplepaMsg();
        //alert("after the DeviceService.paMsg");
    	/**
    	$http.get('/macchina/devices/singleDevice.jss').success(function(data) {
    	      //tmp by sam $scope.devices = data;
    	    });
    	**/
    	//alert("Leave $scope.sendPaMessage");
      }
    
    $scope.setOrderBy = function(col) {
      $scope.orderBy = col;
    }
    $http.get('/macchina/devices/devices.jss').success(function(data) {
      $scope.devices = data;
    });
    /** frequent update of the status
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
