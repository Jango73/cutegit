import QtQuick 2.12

Item {
    id: root

    /*! Lane holding the commit dot */
    property int commitLane: -1

    /*! Vertical segments, one [lane, topAtDot, bottomAtDot] array each */
    property var verticals: []

    /*! Dot-height connections, one [fromLane, toLane, colorLane] array each */
    property var connections: []

    /*! Vertical position of the commit dot, as a fraction of height */
    property real dotFraction: Const.graphDotFraction

    /*! Width of a single lane */
    property int laneWidth: Const.graphLaneWidth

    /*! Maximum lanes drawn, keeps text aligned with a fixed gutter */
    property int maximumLaneCount: Const.graphLaneMaximumCount

    implicitWidth: laneWidth * maximumLaneCount

    /*! Radius of fork and merge elbows */
    property int cornerRadius: Const.graphCornerRadius

    /*! Width of drawn lines */
    property int lineWidth: Const.graphLineWidth

    /*! Radius of the commit dot */
    property int dotRadius: Const.graphDotRadius

    /*! One color per lane, reused cyclically */
    property var laneColors: Const.graphLaneColors

    function laneColor(lane) {
        if (laneColors.length === 0)
            return "black"
        var index = ((lane % laneColors.length) + laneColors.length) % laneColors.length
        return laneColors[index]
    }

    function laneCenter(lane) {
        return lane * laneWidth + laneWidth * 0.5
    }

    /*! Returns true when the lane ends or starts a horizontal connection at the dot */
    function isElbow(lane, columnIndex) {
        for (var connectionIndex = 0; connectionIndex < root.connections.length; connectionIndex++) {
            var connection = root.connections[connectionIndex]
            if (connection[2] === lane && connection[columnIndex] === lane)
                return true
        }
        return false
    }

    Canvas {
        id: graphCanvas
        anchors.fill: parent
        onPaint: {
            var painter = getContext("2d")
            painter.clearRect(0, 0, width, height)
            painter.lineWidth = root.lineWidth
            painter.lineCap = "round"
            var dotY = height * root.dotFraction
            var cornerRadius = Math.min(root.cornerRadius, root.laneWidth * 0.5)

            var verticalIndex = 0
            for (verticalIndex = 0; verticalIndex < root.verticals.length; verticalIndex++) {
                var segment = root.verticals[verticalIndex]
                var lane = segment[0]
                var centerX = root.laneCenter(lane)
                var topY = segment[1] ? dotY + (root.isElbow(lane, 1) ? cornerRadius : 0) : 0
                var bottomY = segment[2] ? dotY - (root.isElbow(lane, 0) ? cornerRadius : 0) : height
                painter.strokeStyle = root.laneColor(lane)
                painter.beginPath()
                painter.moveTo(centerX, topY)
                painter.lineTo(centerX, bottomY)
                painter.stroke()
            }

            var connectionIndex = 0
            for (connectionIndex = 0; connectionIndex < root.connections.length; connectionIndex++) {
                var connection = root.connections[connectionIndex]
                var elbowLane = connection[2]
                var otherLane = (elbowLane === connection[0]) ? connection[1] : connection[0]
                var elbowX = root.laneCenter(elbowLane)
                var otherX = root.laneCenter(otherLane)
                if (elbowX === otherX)
                    continue
                var isFork = (elbowLane === connection[1])
                var direction = (otherX > elbowX) ? 1 : -1
                var horizontalEndX = elbowX + direction * cornerRadius
                var verticalEndY = dotY + (isFork ? cornerRadius : -cornerRadius)
                painter.strokeStyle = root.laneColor(elbowLane)
                painter.beginPath()
                painter.moveTo(otherX, dotY)
                painter.lineTo(horizontalEndX, dotY)
                painter.quadraticCurveTo(elbowX, dotY, elbowX, verticalEndY)
                painter.stroke()
            }

            if (root.commitLane >= 0) {
                painter.fillStyle = root.laneColor(root.commitLane)
                painter.beginPath()
                painter.arc(root.laneCenter(root.commitLane), dotY, root.dotRadius, 0, Math.PI * 2)
                painter.fill()
            }
        }
    }

    onCommitLaneChanged: graphCanvas.requestPaint()
    onVerticalsChanged: graphCanvas.requestPaint()
    onConnectionsChanged: graphCanvas.requestPaint()
    onWidthChanged: graphCanvas.requestPaint()
    onHeightChanged: graphCanvas.requestPaint()
}
