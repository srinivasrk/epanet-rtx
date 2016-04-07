// public/core.js
var rtxLink = angular.module('rtxLink', ['ngRoute']);

rtxLink.config(['$routeProvider', function ($routeProvider) {
    $routeProvider
        .when('/main', {
            templateUrl: 'templates/main.part.html',
            controller: 'MainController'
        })
        .when('/source', {
            templateUrl: 'templates/source.part.html',
            controller: 'SourceController'
        })
        .when('/about', {
            templateUrl: 'templates/about.part.html',
            controller: 'AboutController'
        })
        .when('/series', {
            templateUrl: 'templates/series.part.html',
            controller: 'SeriesController'
        })
        .otherwise({
            redirectTo: '/main'
        });
}]);


rtxLink.controller('OdbcDriverSelectController', function OdbcDriverSelectController($scope, $http) {
    $scope.drivers = ["getting driver list..."];

    $http.get('http://localhost:3131/odbc')
        .then(function (response) {
            $scope.drivers = response.data;
        }, function (response) {
            $scope.drivers = ["failed to get driver list"];
        });

});

rtxLink.controller('MainController', function MainController($scope, $http, $interval) {
    $scope.formData = {};
    $scope.series = [];
    $scope.status = 'Checking';


    $scope.refreshStatus = function() {
        $scope.status = "Checking...";
        $http.get('http://localhost:3131/run')
            .then(function(response) {
                $scope.status = response.data.run ? 'Running' : 'Stopped';
            }, function(response) {
                console.log('Error: ' + response);
                $scope.status = "Error";
            });
    };

    $scope.refreshStatus();
    var refreshInterval = $interval($scope.refreshStatus, 5000);
    $scope.$on('$destroy', function () { $interval.cancel(refreshInterval); });
});

rtxLink.controller('SourceController', function SourceController($scope, $http, $interval) {

    $scope.formData = {};

    $scope.sourceTypes = {
        'ODBC': [
            {
                key:'name',
                text:'Name',
                placeholder:'SCADA Colloquial Name',
                inputType:'text-line'
            },{
                key:'driver',
                text:'ODBC Driver',
                placeholder:'MS-SQL ODBC Provider',
                inputType:'select-line'
            },{
                key:'connectionString',
                text:'Connection',
                placeholder:'SVR=192.168.0.1;blah=blah',
                inputType:'text-line'
            },{
                key:'meta',
                text: 'Name lookup query',
                placeholder: 'SELECT tagname, units FROM tag_list ORDER BY tagname ASC',
                inputType: 'text-line'
            },{
                key: 'range',
                text: 'Data lookup query',
                placeholder: 'SELECT date, value, quality FROM tbl WHERE tagname = ? AND date >= ? AND date <= ? ORDER BY date ASC',
                inputType: 'text-line'
            }
        ],
        'SQLite': [
            {
                key:'name',
                text:'Name',
                placeholder:'User-defined Name',
                inputType:'text-line'
            },
            {
                key:'connectionString',
                text:'File Name',
                placeholder:'tempfile.sqlite',
                inputType:'text-line'
            }
        ],
        'Influx': [
            {
                key:'name',
                text:'Name',
                placeholder:'User-defined Name',
                inputType:'text-line'
            },
            {
                key:'connectionString',
                text:'Connection',
                placeholder:'host=http://flux.citilogics.io&u=user&p=pass',
                inputType:'text-line'
            }
        ]
    };
    $scope.selectedSourceType = 'ODBC';

    $scope.isType = function (typeName) {
        return typeName === $scope.selectedSourceType;
    };
    $scope.setSourceType = function (typeName) {
        $scope.formData = {};
        $scope.selectedSourceType = typeName;
    };

    $scope.save = function () {
        console.log($scope.formData);
    };
});

rtxLink.controller('SeriesController', function SeriesController($scope, $http) {
    $scope.series = [];
    $http.get('http://localhost:3131/series')
        .then(function(response) {
            $scope.series = response.data;
            console.log(response);
        }, function(response) {
            console.log('Error: ' + response);
        });
});

rtxLink.controller('AboutController', function AboutController($scope, $http) {
    $scope.author = 'OWA';
});

rtxLink.controller('HeaderController', function HeaderController($scope, $location) {
    $scope.links = [
        {url:'main', text:'Main'},
        {url:'source', text:'Source'},
        {url:'series', text:'Tags'},
        {url:'about', text:'About'}
    ];

    $scope.isActive = function (viewLocation) {
        return viewLocation === $location.path();
    };
});